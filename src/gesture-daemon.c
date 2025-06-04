#include <linux/input-event-codes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <linux/hidraw.h>
#include <linux/hid.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>
#include <sys/stat.h>


#define TARGET_VENDOR  0x35CC
#define TARGET_PRODUCT 0x0104

#define GESTURE_IDENTIFIER 0x0e
#define SWIPE_VERTICAL_LEFT_EDGE 0x03
#define SWIPE_VERTICAL_RIGHT_EDGE 0x04
#define SWIPE_UP 0x01
#define SWIPE_DOWN 0x02
#define SWIPE_HORIZONTAL_TWO_FINGERS_EDGE 0x0a
#define SWIPE_LEFT 0x03
#define KNOCK_DOUBLE_ONE_KNUCKLE 0x06
#define KNOCK_DOUBLE_TWO_KNUCKLES 0x07
#define CLICK_TOP_LEFT 0x08
#define CLICK_TOP_RIGHT 0x09

volatile int keep_running = 1;

void handle_sigint(int sig) {
    syslog(LOG_INFO, "Received signal %d, shutting down gracefully", sig);
    keep_running = 0;
}

// send key press via uinput
void emit(int ufd, __u16 type, __u16 code, int value) {
    struct input_event ev = {0};
    ev.type = type;
    ev.code = code;
    ev.value = value;
    write(ufd, &ev, sizeof(ev));
}

void send_key_combination(int ufd, int n, ...) {
    va_list keys;
    va_start(keys, n);

    for (int i = 0; i < n; i++) {
        emit(ufd, EV_KEY, va_arg(keys, int), 1);
    }
    emit(ufd, EV_SYN, SYN_REPORT, 0);

    va_start(keys, n);
    for (int i = n - 1; i >= 0; i--) {
        emit(ufd, EV_KEY, va_arg(keys, int), 0);
    }
    emit(ufd, EV_SYN, SYN_REPORT, 0);
    va_end(keys);

}

void send_key(int ufd, int key) {
    send_key_combination(ufd, 1, key);
}

void send_key_combo(int ufd, int modifier, int key) {
    send_key_combination(ufd, 2, modifier, key);
}


// Auto-detection function
char* find_touchpad_hidraw() {
    static char device_path[64];

    for (int i = 0; i < 64; i++) {
        snprintf(device_path, sizeof(device_path), "/dev/hidraw%d", i);

        int fd = open(device_path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) continue;

        struct hidraw_devinfo info;
        if (ioctl(fd, HIDIOCGRAWINFO, &info) == 0) {
            if (info.vendor == TARGET_VENDOR && info.product == TARGET_PRODUCT) {
                close(fd);
                syslog(LOG_INFO, "Found touchpad: %s", device_path);
                return device_path;
            }
        }
        close(fd);
    }

    syslog(LOG_ERR, "Touchpad hidraw device not found");
    return NULL;
}

int setup_uinput_device() {
    int ufd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (ufd < 0) {
        syslog(LOG_ERR, "Failed to open /dev/uinput: %s", strerror(errno));
        exit(1);
    }

    ioctl(ufd, UI_SET_EVBIT, EV_KEY);
    ioctl(ufd, UI_SET_KEYBIT, KEY_BRIGHTNESSUP);
    ioctl(ufd, UI_SET_KEYBIT, KEY_BRIGHTNESSDOWN);
    ioctl(ufd, UI_SET_KEYBIT, KEY_VOLUMEUP);
    ioctl(ufd, UI_SET_KEYBIT, KEY_VOLUMEDOWN);
    ioctl(ufd, UI_SET_KEYBIT, KEY_LEFTALT);
    ioctl(ufd, UI_SET_KEYBIT, KEY_F4);
    ioctl(ufd, UI_SET_KEYBIT, KEY_LEFTMETA); // Super / Windows Key
    ioctl(ufd, UI_SET_KEYBIT, KEY_H);
    ioctl(ufd, UI_SET_KEYBIT, KEY_V); // Super + V for Notification, since KEY_NOTIFICATION_CENTER does not work.
    ioctl(ufd, UI_SET_KEYBIT, KEY_LEFTSHIFT);
    ioctl(ufd, UI_SET_KEYBIT, KEY_SYSRQ); // Print key for screenshot, KEY_PRINT or KEY_SELECTIVE_SCREENSHOT do not work.

    struct uinput_user_dev uidev = {0};
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "TOPS0102:00 35CC:0104 Gesture Control");
    uidev.id.bustype = BUS_I2C;
    uidev.id.vendor  = TARGET_VENDOR;
    uidev.id.product = TARGET_PRODUCT;
    uidev.id.version = 1;

    write(ufd, &uidev, sizeof(uidev));
    ioctl(ufd, UI_DEV_CREATE);

    syslog(LOG_INFO, "Gesture control device created successfully");
    return ufd;
}

int main() {
    // Open syslog for logging
    openlog("gesture-service", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Starting gesture service");

    // Set up signal handlers
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);

    // Auto-detect touchpad
    char *device_path = find_touchpad_hidraw();
    if (!device_path) {
        syslog(LOG_ERR, "Failed to find touchpad");
        return 1;
    }

    // Open HID device
    int hid = open(device_path, O_RDONLY);
    if (hid < 0) {
        syslog(LOG_ERR, "Failed to open %s: %s", device_path, strerror(errno));
        return 1;
    }

    int ufd = setup_uinput_device();

    uint8_t buf[64];
    while (keep_running) {
        // TODO: Check if blocking could be an issue (example: if you try to stop the service, it will only stop once you provide any touchpad input, since only the data is received in the buffer and the blocked passage is left)
        // TODO: During boot, touchpad is needed now due to blocking nature of read: "A stop job is running for Touchpad Gesture daemon". Fix needed, otherwise booting is not possible without touchpad action.
        int n = read(hid, buf, sizeof(buf));
        if (n < 3) continue;

        if (buf[0] == GESTURE_IDENTIFIER) {
            if (buf[1] == SWIPE_VERTICAL_LEFT_EDGE) {
                if (buf[2] == SWIPE_UP) {
                    send_key(ufd, KEY_BRIGHTNESSUP);
                    //syslog(LOG_DEBUG, "Left side swipe up gesture detected");
                } else if (buf[2] == SWIPE_DOWN) {
                    send_key(ufd, KEY_BRIGHTNESSDOWN);
                        //syslog(LOG_DEBUG, "Left side swipe down gesture detected");
                }
            } else if (buf[1] == SWIPE_VERTICAL_RIGHT_EDGE) {
                if (buf[2] == SWIPE_UP) {
                    send_key(ufd, KEY_VOLUMEUP);
                    //syslog(LOG_DEBUG, "Right side swipe up gesture detected");
                } else if (buf[2] == SWIPE_DOWN) {
                    send_key(ufd, KEY_VOLUMEDOWN);
                    //syslog(LOG_DEBUG, "Right side swipe down gesture detected");
                }
            } else if (buf[1] == SWIPE_HORIZONTAL_TWO_FINGERS_EDGE && buf[2] == SWIPE_LEFT) {
                send_key_combo(ufd, KEY_LEFTMETA, KEY_V);
                //syslog(LOG_DEBUG, "Two-finger swipe left from edge gesture detected");
            } else if (buf[1] == KNOCK_DOUBLE_ONE_KNUCKLE) {
                send_key(ufd, KEY_SYSRQ);
                //syslog(LOG_DEBUG, "One-knuckle double knock gesture detected");
            } else if (buf[1] == KNOCK_DOUBLE_TWO_KNUCKLES) {
                send_key_combo(ufd, KEY_LEFTSHIFT, KEY_SYSRQ); // Direct screenshot. Screen capture would need triple combo Shift+Strg+Alt+R in Zorin OS. Anyhow, it is possible to do it from single knuckle double knock through GUI.
                //syslog(LOG_DEBUG, "Two-knuckle double knock gesture detected");
            } else if (buf[1] == CLICK_TOP_LEFT) {
                send_key_combo(ufd, KEY_LEFTMETA, KEY_H);
                //syslog(LOG_DEBUG, "Minimize window gesture detected");
            } else if (buf[1] == CLICK_TOP_RIGHT) {
                send_key_combo(ufd, KEY_LEFTALT, KEY_F4);
                //syslog(LOG_DEBUG, "Close window gesture detected");
            }
        }
    }

    syslog(LOG_INFO, "Shutting down gesture service");
    ioctl(ufd, UI_DEV_DESTROY);
    close(ufd);
    close(hid);
    closelog();
    return 0;
}
