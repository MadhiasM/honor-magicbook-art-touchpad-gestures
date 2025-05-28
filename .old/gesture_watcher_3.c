#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <signal.h>
#include <errno.h>

#define DEVICE "/dev/hidraw1"

volatile int keep_running = 1;

void handle_sigint(int sig) {
    keep_running = 0;
}

// send key press via uinput
void send_key(int ufd, int keycode) {
    struct input_event ev;

    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY;
    ev.code = keycode;
    ev.value = 1;
    write(ufd, &ev, sizeof(ev));

    ev.value = 0;
    write(ufd, &ev, sizeof(ev));

    // sync
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(ufd, &ev, sizeof(ev));
}

int setup_uinput_device() {
    int ufd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (ufd < 0) {
        perror("open /dev/uinput");
        exit(1);
    }

    ioctl(ufd, UI_SET_EVBIT, EV_KEY);
    ioctl(ufd, UI_SET_KEYBIT, KEY_BRIGHTNESSUP);
    ioctl(ufd, UI_SET_KEYBIT, KEY_BRIGHTNESSDOWN);
    ioctl(ufd, UI_SET_KEYBIT, KEY_VOLUMEUP);
    ioctl(ufd, UI_SET_KEYBIT, KEY_VOLUMEDOWN);
    ioctl(ufd, UI_SET_KEYBIT, KEY_MIN_INTERESTING); // placeholder
    //ioctl(ufd, UI_SET_KEYBIT, KEY_MINIMIZE);        // Minimize window, does not exist straightforward in linux
    ioctl(ufd, UI_SET_KEYBIT, KEY_CLOSE);           // Exists, but does not work
    ioctl(ufd, UI_SET_KEYBIT, KEY_SCREENLOCK);      // Use screenlock instead of notification as placeholder

    struct uinput_user_dev uidev = {0};
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "gesture-uinput");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1234;
    uidev.id.product = 0x5678;
    uidev.id.version = 1;

    write(ufd, &uidev, sizeof(uidev));
    ioctl(ufd, UI_DEV_CREATE);

    sleep(1); // give the system time to recognize device
    return ufd;
}

int main() {
    signal(SIGINT, handle_sigint);

    int hid = open(DEVICE, O_RDONLY);
    if (hid < 0) {
        perror("open hidraw");
        return 1;
    }

    int ufd = setup_uinput_device();

    uint8_t buf[64];
    while (keep_running) {
        int n = read(hid, buf, sizeof(buf));
        if (n < 3) continue;

        if (buf[0] == 0x0e) {
            if (buf[1] == 0x03) {
                if (buf[2] == 0x02)
                    send_key(ufd, KEY_BRIGHTNESSDOWN);
                else if (buf[2] == 0x01)
                    send_key(ufd, KEY_BRIGHTNESSUP);
            } else if (buf[1] == 0x04) {
                if (buf[2] == 0x02)
                    send_key(ufd, KEY_VOLUMEDOWN);
                else if (buf[2] == 0x01)
                    send_key(ufd, KEY_VOLUMEUP);
            } else if (buf[1] == 0x0a && buf[2] == 0x03) {
                send_key(ufd, KEY_SCREENLOCK);
            } else if (buf[1] == 0x08) {
                //send_key(ufd, KEY_MINIMIZE);
            } else if (buf[1] == 0x09) {
                send_key(ufd, KEY_CLOSE);
            }
        }
    }

    ioctl(ufd, UI_DEV_DESTROY);
    close(ufd);
    close(hid);
    return 0;
}
