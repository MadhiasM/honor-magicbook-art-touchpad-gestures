#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/hid.h>
#include <linux/slab.h>

#define VENDOR_ID  0x35cc
#define PRODUCT_ID 0x0104

static struct input_dev *gesture_input_dev;

static void send_key_event(unsigned int keycode)
{
    input_report_key(gesture_input_dev, keycode, 1);
    input_sync(gesture_input_dev);
    input_report_key(gesture_input_dev, keycode, 0);
    input_sync(gesture_input_dev);
}

static int gesture_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size)
{
    if (size < 3 || data[0] != 0x0e)
        return 0;

    if (data[1] == 0x03 && data[2] == 0x01) {
        send_key_event(KEY_BRIGHTNESSUP);
    } else if (data[1] == 0x03 && data[2] == 0x02) {
        send_key_event(KEY_BRIGHTNESSDOWN);
    } else if (data[1] == 0x04 && data[2] == 0x01) {
        send_key_event(KEY_VOLUMEUP);
    } else if (data[1] == 0x04 && data[2] == 0x02) {
        send_key_event(KEY_VOLUMEDOWN);
    }

    return 0;
}

static const struct hid_device_id gesture_hid_ids[] = {
    { HID_I2C_DEVICE(VENDOR_ID, PRODUCT_ID) },
    { }
};
MODULE_DEVICE_TABLE(hid, gesture_hid_ids);

static struct hid_driver gesture_driver = {
    .name = "gesture_driver",
    .id_table = gesture_hid_ids,
    .raw_event = gesture_raw_event,
};

static int __init gesture_init(void)
{
    int err;

    gesture_input_dev = input_allocate_device();
    if (!gesture_input_dev)
        return -ENOMEM;

    gesture_input_dev->name = "Gesture Input Device";
    gesture_input_dev->evbit[0] = BIT_MASK(EV_KEY);
    set_bit(KEY_BRIGHTNESSUP, gesture_input_dev->keybit);
    set_bit(KEY_BRIGHTNESSDOWN, gesture_input_dev->keybit);
    set_bit(KEY_VOLUMEUP, gesture_input_dev->keybit);
    set_bit(KEY_VOLUMEDOWN, gesture_input_dev->keybit);

    err = input_register_device(gesture_input_dev);
    if (err)
        return err;

    return hid_register_driver(&gesture_driver);
}

static void __exit gesture_exit(void)
{
    hid_unregister_driver(&gesture_driver);
    input_unregister_device(gesture_input_dev);
}

module_init(gesture_init);
module_exit(gesture_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mathias Moog");
MODULE_DESCRIPTION("Touchpad Gesture -> Fn-Key Kernel Module");
