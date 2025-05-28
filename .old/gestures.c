#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/hid.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/hidraw.h>
#include "gestures.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Honor Touchpad Gestures Project");
MODULE_DESCRIPTION("Kernel-Modul zur Gestenerkennung über HIDRAW für Honor MagicBook Touchpad");
MODULE_VERSION("0.1");

static struct input_dev *gesture_input_dev;

static void send_key(unsigned int keycode)
{
	input_report_key(gesture_input_dev, keycode, 1);
	input_sync(gesture_input_dev);
	input_report_key(gesture_input_dev, keycode, 0);
	input_sync(gesture_input_dev);
}

// HID-Report-Verarbeitung
static int parse_hid_report(const u8 *data, size_t len)
{
	if (len < 3)
		return -EINVAL;

	if (data[0] != 0x0e)
		return 0;  // uninteressant

	u16 gesture_code = (data[1] << 8) | data[2];

	switch (gesture_code) {
	case GESTURE_LEFT_DOWN:
		send_key(KEY_BRIGHTNESSDOWN);
		break;
	case GESTURE_LEFT_UP:
		send_key(KEY_BRIGHTNESSUP);
		break;
	case GESTURE_RIGHT_DOWN:
		send_key(KEY_VOLUMEDOWN);
		break;
	case GESTURE_RIGHT_UP:
		send_key(KEY_VOLUMEUP);
		break;
	default:
		break;
	}
	return 0;
}

// HID-Gerätefilter
static int gestures_raw_event(struct hid_device *hdev, struct hid_report *report,
			      u8 *data, int size)
{
	if (hdev->vendor != VENDOR_ID || hdev->product != PRODUCT_ID)
		return 0;

	parse_hid_report(data, size);
	return 0;
}

static int gestures_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int ret;

	ret = hid_parse(hdev);
	if (ret) {
		pr_err("hid_parse failed\n");
		return ret;
	}

	ret = hid_hw_start(hdev, HID_CONNECT_HIDRAW);
	if (ret) {
		pr_err("hid_hw_start failed\n");
		return ret;
	}

	return 0;
}

static void gestures_remove(struct hid_device *hdev)
{
	hid_hw_stop(hdev);
}

static const struct hid_device_id gestures_devices[] = {
	{ HID_I2C_DEVICE(VENDOR_ID, PRODUCT_ID) },
	{}
};
MODULE_DEVICE_TABLE(hid, gestures_devices);

static struct hid_driver gestures_driver = {
	.name         = "touchpad_gestures",
	.id_table     = gestures_devices,
	.probe        = gestures_probe,
	.remove       = gestures_remove,
	.raw_event    = gestures_raw_event,
};

static int __init gestures_init(void)
{
	int ret;

	// Virtuelles Input Device für Fn-Key Events
	gesture_input_dev = input_allocate_device();
	if (!gesture_input_dev)
		return -ENOMEM;

	gesture_input_dev->name = "touchpad-gesture-input";
	gesture_input_dev->evbit[0] = BIT_MASK(EV_KEY);
	set_bit(KEY_BRIGHTNESSDOWN, gesture_input_dev->keybit);
	set_bit(KEY_BRIGHTNESSUP, gesture_input_dev->keybit);
	set_bit(KEY_VOLUMEDOWN, gesture_input_dev->keybit);
	set_bit(KEY_VOLUMEUP, gesture_input_dev->keybit);

	ret = input_register_device(gesture_input_dev);
	if (ret) {
		input_free_device(gesture_input_dev);
		return ret;
	}

	ret = hid_register_driver(&gestures_driver);
	if (ret) {
		input_unregister_device(gesture_input_dev);
		return ret;
	}

	pr_info("touchpad_gestures: geladen\n");
	return 0;
}

static void __exit gestures_exit(void)
{
	hid_unregister_driver(&gestures_driver);
	input_unregister_device(gesture_input_dev);
	pr_info("touchpad_gestures: entladen\n");
}

module_init(gestures_init);
module_exit(gestures_exit);
