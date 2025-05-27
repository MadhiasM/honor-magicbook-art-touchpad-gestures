#include <linux/module.h>
#include <linux/init.h>
#include <linux/hid.h>
#include <linux/input.h>
#include <linux/slab.h>

#define VENDOR_ID  0x35cc
#define PRODUCT_ID 0x0104

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OpenAI & User");
MODULE_DESCRIPTION("Touchpad Gesture Handler for Brightness/Volume");
MODULE_VERSION("0.1");

struct gesture_drv {
	struct hid_device *hdev;
	struct input_dev *input;
};

static int gesture_raw_event(struct hid_device *hdev, struct hid_report *report,
                              u8 *data, int size)
{
	struct gesture_drv *drv = hid_get_drvdata(hdev);

	if (size < 3)
		return 0;

	if (data[0] != 0x0e)
		return 0;

	// Check for known gestures
	if (data[1] == 0x03 && data[2] == 0x01) {
		input_report_key(drv->input, KEY_BRIGHTNESSUP, 1);
		input_sync(drv->input);
		input_report_key(drv->input, KEY_BRIGHTNESSUP, 0);
		input_sync(drv->input);
	} else if (data[1] == 0x03 && data[2] == 0x02) {
		input_report_key(drv->input, KEY_BRIGHTNESSDOWN, 1);
		input_sync(drv->input);
		input_report_key(drv->input, KEY_BRIGHTNESSDOWN, 0);
		input_sync(drv->input);
	} else if (data[1] == 0x04 && data[2] == 0x01) {
		input_report_key(drv->input, KEY_VOLUMEUP, 1);
		input_sync(drv->input);
		input_report_key(drv->input, KEY_VOLUMEUP, 0);
		input_sync(drv->input);
	} else if (data[1] == 0x04 && data[2] == 0x02) {
		input_report_key(drv->input, KEY_VOLUMEDOWN, 1);
		input_sync(drv->input);
		input_report_key(drv->input, KEY_VOLUMEDOWN, 0);
		input_sync(drv->input);
	}

	return 0;
}

static int gesture_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int ret;
	struct gesture_drv *drv;

	drv = devm_kzalloc(&hdev->dev, sizeof(*drv), GFP_KERNEL);
	if (!drv)
		return -ENOMEM;

	hid_set_drvdata(hdev, drv);
	drv->hdev = hdev;

	ret = hid_parse(hdev);
	if (ret)
		return ret;

	ret = hid_hw_start(hdev, HID_CONNECT_HIDRAW);
	if (ret)
		return ret;

	// Create virtual input device
	drv->input = devm_input_allocate_device(&hdev->dev);
	if (!drv->input)
		return -ENOMEM;

	drv->input->name = "Touchpad Gesture Keys";
	drv->input->phys = "touchpad/gestures";
	drv->input->id.bustype = BUS_I2C;
	drv->input->dev.parent = &hdev->dev;

	set_bit(EV_KEY, drv->input->evbit);
	set_bit(KEY_BRIGHTNESSUP, drv->input->keybit);
	set_bit(KEY_BRIGHTNESSDOWN, drv->input->keybit);
	set_bit(KEY_VOLUMEUP, drv->input->keybit);
	set_bit(KEY_VOLUMEDOWN, drv->input->keybit);

	ret = input_register_device(drv->input);
	if (ret)
		return ret;

	return 0;
}

static void gesture_remove(struct hid_device *hdev)
{
	hid_hw_stop(hdev);
}

static const struct hid_device_id gesture_devices[] = {
	{ HID_I2C_DEVICE(VENDOR_ID, PRODUCT_ID) },
	{ }
};
MODULE_DEVICE_TABLE(hid, gesture_devices);

static struct hid_driver gesture_driver = {
	.name = "touchpad_gesture_driver",
	.id_table = gesture_devices,
	.probe = gesture_probe,
	.remove = gesture_remove,
	.raw_event = gesture_raw_event,
};

module_hid_driver(gesture_driver);
