#define _GNU_SOURCE

#include "touchpad-config.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/hidraw.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static const struct touchpad_setting SETTINGS[] = {
    {"sensitivity", 0x01, 0, 1},
    {"vibration_intensity", 0x02, 0, 2},
    {"press_text", 0x03, 0, 1},
    {"press_picture", 0x04, 0, 1},
    {"three_finger_drag", 0x05, 0, 1},
    {"mouse_like_mode", 0x06, 0, 1},
    {"edge_brightness", 0x07, 0, 1},
    {"edge_volume", 0x08, 0, 1},
    {"edge_control_center", 0x09, 0, 1},
    {"edge_close_or_minimize", 0x0A, 0, 1},
    {"knuckle_screenshot", 0x0B, 0, 1},
    {"knuckle_screen_record", 0x0C, 0, 1},
};

static void set_error(char *error, size_t error_size, const char *format, ...)
{
    va_list args;

    if (error_size == 0)
        return;
    va_start(args, format);
    vsnprintf(error, error_size, format, args);
    va_end(args);
}

size_t touchpad_setting_count(void)
{
    return sizeof(SETTINGS) / sizeof(SETTINGS[0]);
}

const struct touchpad_setting *touchpad_setting_at(size_t index)
{
    if (index >= touchpad_setting_count())
        return NULL;
    return &SETTINGS[index];
}

const struct touchpad_setting *touchpad_find_setting(const char *name)
{
    size_t i;

    if (!name)
        return NULL;

    for (i = 0; i < touchpad_setting_count(); i++) {
        if (strcmp(name, SETTINGS[i].name) == 0)
            return &SETTINGS[i];
    }
    return NULL;
}

int touchpad_make_report(const struct touchpad_setting *setting, int value,
                         uint8_t report[HONOR_TOUCHPAD_REPORT_SIZE])
{
    if (!setting || !report || value < setting->minimum ||
        value > setting->maximum)
        return -1;

    memset(report, 0, HONOR_TOUCHPAD_REPORT_SIZE);
    report[0] = HONOR_TOUCHPAD_REPORT_ID;
    report[1] = setting->command;
    report[2] = (uint8_t)value;
    return 0;
}

int touchpad_find_device(char *path, size_t path_size)
{
    int i;

    if (!path || path_size == 0)
        return -1;
    for (i = 0; i < 64; i++) {
        int fd;
        struct hidraw_devinfo info;

        snprintf(path, path_size, "/dev/hidraw%d", i);
        fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd < 0)
            continue;
        memset(&info, 0, sizeof(info));
        if (ioctl(fd, HIDIOCGRAWINFO, &info) == 0 &&
            info.vendor == HONOR_TOUCHPAD_VENDOR &&
            info.product == HONOR_TOUCHPAD_PRODUCT) {
            close(fd);
            return 0;
        }
        close(fd);
    }
    path[0] = '\0';
    return -1;
}

int touchpad_set(const char *setting_name, int value, char *error,
                 size_t error_size)
{
    const struct touchpad_setting *setting;
    uint8_t report[HONOR_TOUCHPAD_REPORT_SIZE];
    char path[64];
    ssize_t written;
    int reports = 1;
    int report_index;
    int fd;

    setting = touchpad_find_setting(setting_name);
    if (!setting) {
        set_error(error, error_size, "unknown setting: %s", setting_name);
        return -1;
    }
    if (touchpad_make_report(setting, value, report) < 0) {
        set_error(error, error_size, "%s must be between %d and %d",
                  setting->name, setting->minimum, setting->maximum);
        return -1;
    }
    if (strcmp(setting->name, "three_finger_drag") == 0)
        reports = 2;
    if (touchpad_find_device(path, sizeof(path)) < 0) {
        set_error(error, error_size, "Honor touchpad 35cc:0104 not found");
        return -1;
    }

    fd = open(path, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        set_error(error, error_size, "cannot open %s: %s", path,
                  strerror(errno));
        return -1;
    }
    for (report_index = 0; report_index < reports; report_index++) {
        if (report_index == 1)
            report[1] = 0x11;
        written = write(fd, report, sizeof(report));
        if (written != (ssize_t)sizeof(report)) {
            set_error(error, error_size, "cannot write %s: %s", path,
                      written < 0 ? strerror(errno) : "short write");
            close(fd);
            return -1;
        }
    }
    close(fd);
    return 0;
}
