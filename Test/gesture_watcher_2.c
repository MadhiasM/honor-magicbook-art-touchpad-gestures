#include <fcntl.h>
#include <linux/uinput.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void send_key(int fd, int keycode) {
    struct input_event ev;

    memset(&ev, 0, sizeof(ev));
    gettimeofday(&ev.time, NULL);
    ev.type = EV_KEY;
    ev.code = keycode;
    ev.value = 1;
    write(fd, &ev, sizeof(ev)); // key down

    ev.value = 0;
    write(fd, &ev, sizeof(ev)); // key up

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(ev)); // sync
}

int main() {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) return 1;

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, KEY_BRIGHTNESSDOWN);
    ioctl(fd, UI_SET_KEYBIT, KEY_BRIGHTNESSUP);
    ioctl(fd, UI_SET_KEYBIT, KEY_VOLUMEUP);
    ioctl(fd, UI_SET_KEYBIT, KEY_VOLUMEDOWN);

    struct uinput_user_dev uidev = {0};
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "gesture-virtual-keys");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1234;
    uidev.id.product = 0x5678;

    write(fd, &uidev, sizeof(uidev));
    ioctl(fd, UI_DEV_CREATE);

    sleep(1); // wait for dev node

    // Testevent: Lautstärke runter
    send_key(fd, KEY_VOLUMEDOWN);

    // TODO: Binde hier deine Geste → Keycode Logik ein

    sleep(1);
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return 0;
}
