#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/hid.h>

#define DRIVER_DESC "Minimal Touchpad Gesture Driver"
#define VENDOR_ID  0x35CC
#define PRODUCT_ID 0x0104

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dein Name");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_ALIAS("hid:b0018g*v000035CCp00000104");

// ----- Probe -----
static struct input_dev *input_dev;

static int touchpad_gestures_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int ret;

    printk(KERN_INFO "touchpad_gestures: Probe gestartet für Gerät %s\n", hdev->name);

    ret = hid_parse(hdev);
    if (ret) {
        printk(KERN_ERR "touchpad_gestures: hid_parse() fehlgeschlagen\n");
        return ret;
    }

    ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (ret) {
        printk(KERN_ERR "touchpad_gestures: hid_hw_start() fehlgeschlagen\n");
        return ret;
    }

    // Virtuelles Input-Gerät anlegen
    input_dev = input_allocate_device();
    if (!input_dev) {
        printk(KERN_ERR "touchpad_gestures: input device allocation failed\n");
        hid_hw_stop(hdev);
        return -ENOMEM;
    }

    input_dev->name = "touchpad-gesture-input";
    input_dev->id.bustype = BUS_VIRTUAL;

    set_bit(EV_KEY, input_dev->evbit);
    set_bit(KEY_VOLUMEUP, input_dev->keybit);
    set_bit(KEY_VOLUMEDOWN, input_dev->keybit);
    set_bit(KEY_BRIGHTNESSUP, input_dev->keybit);
    set_bit(KEY_BRIGHTNESSDOWN, input_dev->keybit);

    ret = input_register_device(input_dev);
    if (ret) {
        printk(KERN_ERR "touchpad_gestures: input device registration failed: %d\n", ret);
        input_free_device(input_dev);
        hid_hw_stop(hdev);
        return ret;
    }

    printk(KERN_INFO "touchpad_gestures: Gerät erfolgreich gestartet und Input-Device registriert\n");

    return 0;
}

// ----- Remove -----
static void touchpad_gestures_remove(struct hid_device *hdev)
{
    printk(KERN_INFO "touchpad_gestures: remove() aufgerufen für %s\n", hdev->name);

    if (input_dev) {
        input_unregister_device(input_dev);
        input_dev = NULL;
    }

    hid_hw_stop(hdev);
}

// ----- Geräte-IDs -----
static const struct hid_device_id touchpad_gestures_devices[] = {
    { HID_I2C_DEVICE(VENDOR_ID, PRODUCT_ID) },
    { }
};
MODULE_DEVICE_TABLE(hid, touchpad_gestures_devices);

// ----- HID-Treiberstruktur -----
static struct hid_driver touchpad_gestures_driver = {
    .name = "touchpad_gestures",
    .id_table = touchpad_gestures_devices,
    .probe = touchpad_gestures_probe,
    .remove = touchpad_gestures_remove,
};

// ----- Initialisierung -----
static int __init touchpad_gestures_init(void)
{
    printk(KERN_INFO "touchpad_gestures: Modul geladen\n");
    return hid_register_driver(&touchpad_gestures_driver);
}

static void __exit touchpad_gestures_exit(void)
{
    printk(KERN_INFO "touchpad_gestures: Modul entfernt\n");
    hid_unregister_driver(&touchpad_gestures_driver);
}

module_init(touchpad_gestures_init);
module_exit(touchpad_gestures_exit);
