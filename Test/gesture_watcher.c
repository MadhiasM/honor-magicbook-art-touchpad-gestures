#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define DEVICE "/dev/hidraw1"

void handle_gesture(unsigned char *data, int len) {
    if (len < 3 || data[0] != 0x0e) return;

    unsigned char g1 = data[1];
    unsigned char g2 = data[2];

    if (g1 == 0x03 && g2 == 0x02) {
        printf("⬇️  Linke Seite runter (Helligkeit -)\n");
        system("brightnessctl set 10%-");
    } else if (g1 == 0x03 && g2 == 0x01) {
        printf("⬆️  Linke Seite rauf (Helligkeit +)\n");
        system("brightnessctl set +10%");
    } else if (g1 == 0x04 && g2 == 0x02) {
        printf("🔉 Rechte Seite runter (Volume -)\n");
        system("pactl set-sink-volume @DEFAULT_SINK@ -5%");
    } else if (g1 == 0x04 && g2 == 0x01) {
        printf("🔊 Rechte Seite rauf (Volume +)\n");
        system("pactl set-sink-volume @DEFAULT_SINK@ +5%");
    } else if (g1 == 0x0a && g2 == 0x03) {
        printf("📨 Zwei Finger von rechts nach links (Notifications)\n");
        system("dbus-send --session --dest=org.gnome.Shell /org/gnome/Shell org.gnome.Shell.ShowMessageTray");
    } else if (g1 == 0x08) {
        printf("🗕 Linke obere Ecke (Fenster minimieren)\n");
        system("xdotool getactivewindow windowminimize");
    } else if (g1 == 0x09) {
        printf("❌ Rechte obere Ecke (Fenster schließen)\n");
        system("xdotool getactivewindow windowkill");
    } else {
        printf("Unbekanntes Muster: %02x %02x %02x\n", data[0], data[1], data[2]);
    }
}

int main() {
    int fd = open(DEVICE, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    unsigned char buf[64];
    printf("✨ Lausche auf HID-Gesten auf %s ...\n", DEVICE);

    while (1) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n > 0) {
            handle_gesture(buf, n);
        } else if (n < 0) {
            perror("read");
            break;
        }
    }

    close(fd);
    return 0;
}
