#ifndef HONOR_TOUCHPAD_CONFIG_H
#define HONOR_TOUCHPAD_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#define HONOR_TOUCHPAD_VENDOR 0x35CC
#define HONOR_TOUCHPAD_PRODUCT 0x0104
#define HONOR_TOUCHPAD_REPORT_ID 0x0E
#define HONOR_TOUCHPAD_REPORT_SIZE 9

struct touchpad_setting {
    const char *name;
    uint8_t command;
    int minimum;
    int maximum;
};

const struct touchpad_setting *touchpad_find_setting(const char *name);
size_t touchpad_setting_count(void);
const struct touchpad_setting *touchpad_setting_at(size_t index);
int touchpad_make_report(const struct touchpad_setting *setting, int value,
                         uint8_t report[HONOR_TOUCHPAD_REPORT_SIZE]);
int touchpad_find_device(char *path, size_t path_size);
int touchpad_set(const char *setting_name, int value, char *error,
                 size_t error_size);

#endif
