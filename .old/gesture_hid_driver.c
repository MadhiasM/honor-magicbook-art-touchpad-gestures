// file: gesture_hid_driver.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hid.h>
#include <linux/input.h>

#define DRIVER_NAME "gesture_hid"

static const struct hid_device_id gesture_devices[] = {
    { HID_I2C_DEVICE(0x35cc, 0x0104) }, // Ersetze durch deine Vendor/Product-ID
    { }
};
MODULE_DEVICE_TABLE(hid, gesture_devices);

struct gesture_data {
    struct input_dev *input;
};

static int gesture_raw_event(struct hid_device *hdev, struct hid_report *report,
                             u8 *data, int size)
{
    struct gesture_data *gdata = hid_get_drvdata(hdev);

    if (size < 3)
        return 0;

    if (data[0] == 0x0e) {
        // Helligkeit
        if (data[1] == 0x03) {
            if (data[2] == 0x01) {
                input_report_key(gdata->input, KEY_BRIGHTNESSUP, 1);
                input_sync(gdata->input);
                input_report_key(gdata->input, KEY_BRIGHTNESSUP, 0);
                input_sync(gdata->input);
            } else if (data[2] == 0x02) {
                input_report_key(gdata->input, KEY_BRIGHTNESSDOWN, 1);
                input_sync(gdata->input);
                input_report_key(gdata->input, KEY_BRIGHTNESSDOWN, 0);
                input_sync(gdata->input);
            }
        }

        // Lautstärke
        if (data[1] == 0x04) {
            if (data[2] == 0x01) {
                input_report_key(gdata->input, KEY_VOLUMEUP, 1);
                input_sync(gdata->input);
                input_report_key(gdata->input, KEY_VOLUMEUP, 0);
                input_sync(gdata->input);
            } else if (data[2] == 0x02) {
                input_report_key(gdata->input, KEY_VOLUMEDOWN, 1);
                input_sync(gdata->input);
                input_report_key(gdata->input, KEY_VOLUMEDOWN, 0);
                input_sync(gdata->input);
            }
        }
    }

    return 0;
}

static int gesture_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int ret;
    struct gesture_data *gdata;
    struct input_dev *input;

    gdata = devm_kzalloc(&hdev->dev, sizeof(*gdata), GFP_KERNEL);
    if (!gdata)
        return -ENOMEM;

    input = devm_input_allocate_device(&hdev->dev);
    if (!input)
        return -ENOMEM;

    gdata->input = input;

    input->name = "Gesture HID Input";
    input->phys = "gesture/input0";
    input->id.bustype = hdev->bus;
    input->dev.parent = &hdev->dev;

    set_bit(EV_KEY, input->evbit);
    set_bit(KEY_BRIGHTNESSUP, input->keybit);
    set_bit(KEY_BRIGHTNESSDOWN, input->keybit);
    set_bit(KEY_VOLUMEUP, input->keybit);
    set_bit(KEY_VOLUMEDOWN, input->keybit);

    hid_set_drvdata(hdev, gdata);

    ret = input_register_device(input);
    if (ret)
        return ret;

    ret = hid_parse(hdev);
    if (ret)
        return ret;

    ret = hid_hw_start(hdev, HID_HW_RAW);
    return ret;
}

static void gesture_remove(struct hid_device *hdev)
{
    hid_hw_stop(hdev);
}

static struct hid_driver gesture_driver = {
    .name = DRIVER_NAME,
    .id_table = gesture_devices,
    .probe = gesture_probe,
    .remove = gesture_remove,
    .raw_event = gesture_raw_event,
};

module_hid_driver(gesture_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Du");
MODULE_DESCRIPTION("Custom HID Gesture Driver");
