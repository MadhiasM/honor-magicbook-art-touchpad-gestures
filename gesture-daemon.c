#include <linux/input-event-codes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>
#include <sys/stat.h>

#define DEVICE "/dev/hidraw1"

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

void send_key(int ufd, int keycode) {
    emit(ufd, EV_KEY, keycode, 1);
    emit(ufd, EV_KEY, keycode, 0);
    emit(ufd, EV_SYN, SYN_REPORT, 0);
}

void send_key_combo(int ufd, int modifier, int key) {
    emit(ufd, EV_KEY, modifier, 1);
    emit(ufd, EV_KEY, key, 1);
    emit(ufd, EV_KEY, key, 0);
    emit(ufd, EV_KEY, modifier, 0);
    emit(ufd, EV_SYN, SYN_REPORT, 0);
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
    ioctl(ufd, UI_SET_KEYBIT, KEY_LEFTMETA);
    ioctl(ufd, UI_SET_KEYBIT, KEY_H);
    ioctl(ufd, UI_SET_KEYBIT, KEY_V);

    struct uinput_user_dev uidev = {0};
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "TOPS0102:00 35CC:0104 Gesture Control");
    uidev.id.bustype = BUS_I2C;
    uidev.id.vendor  = 0x35CC;
    uidev.id.product = 0x0104;
    uidev.id.version = 1;

    write(ufd, &uidev, sizeof(uidev));
    ioctl(ufd, UI_DEV_CREATE);

    sleep(1); // give the system time to recognize device
    syslog(LOG_INFO, "uinput device created successfully");
    return ufd;
}

int main() {
    // Open syslog for logging
    openlog("gesture-service", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Starting gesture service");

    // Set up signal handlers
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);

    // Open HID device
    int hid = open(DEVICE, O_RDONLY);
    if (hid < 0) {
        syslog(LOG_ERR, "Failed to open %s: %s", DEVICE, strerror(errno));
        return 1;
    }

    int ufd = setup_uinput_device();

    uint8_t buf[64];
    while (keep_running) {
        // TODO: Check if blocking could be an issue (example: if you try to stop the service, it will only stop once you provide any touchpad input, since only the data is received in the buffer and the blocked passage is left)
        // TODO: During boot, touchpad is needed now due to blocking nature of read: "A stop job is running for Touchpad Gesture daemon". Fix needed, otherwise booting is not possible without touchpad action.
        int n = read(hid, buf, sizeof(buf));
        if (n < 3) continue;

        if (buf[0] == 0x0e) {
            if (buf[1] == 0x03) {
                if (buf[2] == 0x02) {
                    send_key(ufd, KEY_BRIGHTNESSDOWN);
                        syslog(LOG_DEBUG, "Brightness down gesture detected");
                } else if (buf[2] == 0x01) {
                    send_key(ufd, KEY_BRIGHTNESSUP);
                    syslog(LOG_DEBUG, "Brightness up gesture detected");
                }
            } else if (buf[1] == 0x04) {
                if (buf[2] == 0x02) {
                    send_key(ufd, KEY_VOLUMEDOWN);
                    syslog(LOG_DEBUG, "Volume down gesture detected");
                } else if (buf[2] == 0x01) {
                    send_key(ufd, KEY_VOLUMEUP);
                    syslog(LOG_DEBUG, "Volume up gesture detected");
                }
            } else if (buf[1] == 0x0a && buf[2] == 0x03) {
                send_key_combo(ufd, KEY_LEFTMETA, KEY_V);
                syslog(LOG_DEBUG, "Notification panel gesture detected");
            } else if (buf[1] == 0x08) {
                send_key_combo(ufd, KEY_LEFTMETA, KEY_H);
                syslog(LOG_DEBUG, "Minimize window gesture detected");
            } else if (buf[1] == 0x09) {
                send_key_combo(ufd, KEY_LEFTALT, KEY_F4);
                syslog(LOG_DEBUG, "Close window gesture detected");
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
