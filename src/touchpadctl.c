#include "touchpad-config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *program)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s list\n"
            "  %s set SETTING VALUE\n\n"
            "Boolean settings use 0/1. vibration_intensity uses 0/1/2.\n",
            program, program);
}

int main(int argc, char **argv)
{
    char error[256];
    char *end;
    long value;
    size_t i;

    if (argc == 2 && strcmp(argv[1], "list") == 0) {
        for (i = 0; i < touchpad_setting_count(); i++) {
            const struct touchpad_setting *setting =
                touchpad_setting_at(i);
            printf("%s %d..%d\n", setting->name, setting->minimum,
                   setting->maximum);
        }
        return 0;
    }
    if (argc != 4 || strcmp(argv[1], "set") != 0) {
        usage(argv[0]);
        return 2;
    }

    errno = 0;
    value = strtol(argv[3], &end, 10);
    if (errno != 0 || end == argv[3] || *end != '\0') {
        fprintf(stderr, "invalid value: %s\n", argv[3]);
        return 2;
    }
    if (touchpad_set(argv[2], (int)value, error, sizeof(error)) < 0) {
        fprintf(stderr, "touchpadctl: %s\n", error);
        return 1;
    }
    printf("sent %s=%ld\n", argv[2], value);
    return 0;
}
