#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("Touchpad Gesture Input Handler");

// Vendor/Product ID deines Touchpads:
#define VENDOR_ID  0x35CC
#define PRODUCT_ID 0x0104

static struct input_handle *gesture_handle;
static struct input_dev *gesture_input_dev;

// IDs für Input Handler (Filter auf dein Touchpad)
static const struct input_device_id gesture_ids[] = {
    {
        .vendor = VENDOR_ID,
        .product = PRODUCT_ID,
        .flags = INPUT_DEVICE_ID_MATCH_VENDOR | INPUT_DEVICE_ID_MATCH_PRODUCT,
    },
    { }, // Terminator
};

// Hilfsfunktion: Key event auslösen (Hoch + Runter)
static void send_key_event(struct input_dev *dev, unsigned int key) {
    input_report_key(dev, key, 1);
    input_sync(dev);
    input_report_key(dev, key, 0);
    input_sync(dev);
}

// Event Callback
static void gesture_input_event(struct input_handle *handle,
                                unsigned int type, unsigned int code, int value)
{
    // Wir wollen nur EV_MSC/EV_KEY/EV_ABS Events genauer ansehen
    if (type == EV_MSC || type == EV_KEY || type == EV_ABS) {
        printk(KERN_INFO "gesture_event: type=%u code=%u value=%d\n", type, code, value);
    }

    // Hier müsstest du deine eigenen 3-Byte Muster erkennen.
    // Zum Beispiel: Bei bestimmtem Keycode 0x03 und Value 0x01 für "Helligkeit hoch":
    // (ACHTUNG: Die Rohdaten kommen nicht als einzelne Bytes so rein, 
    // sondern als Events. Hier müsstest du eigentlich einen Puffer bauen
    // und deine 3-Byte Gesten logik implementieren.)

    // Dummy: Beispiel nur Lautstärke hoch simulieren bei Taste KEY_VOLUMEUP
    if (type == EV_KEY && code == KEY_VOLUMEUP && value == 1) {
        printk(KERN_INFO "Gesture detected: Lautstärke hoch\n");
        // Hier könntest du z.B. weitere Key Events senden
    }
}

// Connect Callback
static int gesture_input_connect(struct input_handler *handler,
                                 struct input_dev *dev,
                                 const struct input_device_id *id)
{
    int error;

    printk(KERN_INFO "Gesture handler connecting to device: %s\n", dev->name);

    gesture_handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
    if (!gesture_handle)
        return -ENOMEM;

    gesture_handle->dev = dev;
    gesture_handle->handler = handler;
    gesture_handle->name = "touchpad_gesture_handle";

    error = input_register_handle(gesture_handle);
    if (error) {
        kfree(gesture_handle);
        return error;
    }

    error = input_open_device(gesture_handle, dev);
    if (error) {
        input_unregister_handle(gesture_handle);
        kfree(gesture_handle);
        return error;
    }

    gesture_input_dev = dev;

    printk(KERN_INFO "Gesture handler connected.\n");
    return 0;
}

static void gesture_input_disconnect(struct input_handle *handle)
{
    printk(KERN_INFO "Gesture handler disconnected.\n");
    input_close_device(handle);
    input_unregister_handle(handle);
    kfree(handle);
}

// Input Handler Definition
static struct input_handler gesture_input_handler = {
    .event = gesture_input_event,
    .connect = gesture_input_connect,
    .disconnect = gesture_input_disconnect,
    .name = "touchpad_gestures_handler",
    .id_table = gesture_ids,
};

static int __init gesture_init(void)
{
    printk(KERN_INFO "Loading touchpad gesture handler module...\n");
    return input_register_handler(&gesture_input_handler);
}

static void __exit gesture_exit(void)
{
    printk(KERN_INFO "Unloading touchpad gesture handler module...\n");
    input_unregister_handler(&gesture_input_handler);
}

module_init(gesture_init);
module_exit(gesture_exit);
