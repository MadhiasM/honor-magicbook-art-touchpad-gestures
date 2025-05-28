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

void send_key(int ufd, int keycode) {
    struct input_event ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY;
    ev.code = keycode;
    ev.value = 1;
    write(ufd, &ev, sizeof(ev));

    ev.value = 0;
    write(ufd, &ev, sizeof(ev));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(ufd, &ev, sizeof(ev));
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
    ioctl(ufd, UI_SET_KEYBIT, KEY_MIN_INTERESTING);
    //ioctl(ufd, UI_SET_KEYBIT, KEY_MINIMIZE);        // Fenster minimieren
    ioctl(ufd, UI_SET_KEYBIT, KEY_CLOSE);
    ioctl(ufd, UI_SET_KEYBIT, KEY_SCREENLOCK);

    struct uinput_user_dev uidev = {0};
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "gesture-service");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1234;
    uidev.id.product = 0x5678;
    uidev.id.version = 1;

    write(ufd, &uidev, sizeof(uidev));
    ioctl(ufd, UI_DEV_CREATE);

    sleep(1);
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
    syslog(LOG_INFO, "Opened HID device %s", DEVICE);

    int ufd = setup_uinput_device();

    uint8_t buf[64];
    while (keep_running) {
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
                send_key(ufd, KEY_SCREENLOCK);
                syslog(LOG_DEBUG, "Notification panel gesture detected");
            } else if (buf[1] == 0x09) {
                send_key(ufd, KEY_CLOSE);
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
