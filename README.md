# Honor Magicbook Art Touchpad Gestures

## Content
This will replicate the special gestures on the Honor Magicbook Art Touchpad

### Brightness control
Vertical Swipe on the left
### Volume control
Vertical Swipe on the right

### Notification panel
Two-finger swipe leftwards from the right edge

### Minimize Window
Click on top left of touchpad
### Close Window
Click on top right of touchpad

## Adjustments
Before deploying the service in linux, make sure the gesture you want to use are enabled in **Windows**. This is due to the fact that the configuration of enabled gestures is stored on the touchpad firmware itself. Changing these settings can only be done in Windows at the moment.

Depending on your distro, you might need to adjust some of the key combos for closing/minimizing windows and opening the notification panel. Current implementation for closing window:
```c
ioctl(ufd, UI_SET_KEYBIT, KEY_LEFTALT);
ioctl(ufd, UI_SET_KEYBIT, KEY_F4);

(ufd, KEY_LEFTALT, KEY_F4);
```

## Installation
### 1: Clone repo
```bash
git clone https://github.com/MadhiasM/honor-magicbook-art-touchpad-gestures
```

### 2: Compile service

```bash
# Compile service
gcc -o gesture-daemon gesture-daemon.c
```

### 3: Deploy service

```bash
# Copy service to system location and make executable
sudo cp gesture-daemon /usr/local/bin/
sudo chmod +x /usr/local/bin/gesture-daemon

# Copy service file to systemd directory
sudo cp gesture-daemon.service /etc/systemd/system/

# Reload systemd to recognize new service
sudo systemctl daemon-reload

# Enable service to start at boot
sudo systemctl enable gesture-daemon

# Start service
sudo systemctl start gesture-daemon

# Check service status
sudo systemctl status gesture-daemon

# Check service events for Gesture control
sudo evtest

```

Your gesture service should now be running as a proper systemd service with logging and automatic startup

## Monitoring
After 5 minutes of usage, it only used the CPU for <50 ms and takes up <300 kB of memory, as per systemctl status:
```
● gesture-daemon.service - Touchpad Gesture Daemon
     Loaded: loaded (/etc/systemd/system/gesture-daemon.service; enabled; vendor preset: enabled)
     Active: active (running) since Thu 2025-05-29 12:13:32 CEST; 5min ago
       Docs: man:gesture-daemon(1)
   Main PID: 9389 (gesture-daemon)
      Tasks: 1 (limit: 18336)
     Memory: 280.0K
        CPU: 39ms
     CGroup: /system.slice/gesture-daemon.service
             └─9389 /usr/local/bin/gesture-daemon

Mai 29 12:13:32 MRA-XXX systemd[1]: Started Touchpad Gesture Daemon.
Mai 29 12:13:32 MRA-XXX gesture-service[9389]: Starting gesture service
Mai 29 12:13:32 MRA-XXX gesture-service[9389]: Found touchpad: /dev/hidraw1
Mai 29 12:13:32 MRA-XXX gesture-service[9389]: Gesture control device created successfully
```

Events visible like this, as per evtest:
```
No device specified, trying to scan all of /dev/input/event*
Available devices:
/dev/input/event0:	Lid Switch
/dev/input/event1:	Power Button
/dev/input/event2:	AT Translated Set 2 keyboard
/dev/input/event3:	Huawei WMI hotkeys
/dev/input/event4:	FTSC1000:00 2808:5662
/dev/input/event5:	TOPS0102:00 35CC:0104 Consumer Control
/dev/input/event6:	TOPS0102:00 35CC:0104 Touchpad
/dev/input/event7:	sof-hda-dsp Headphone
/dev/input/event8:	Video Bus
/dev/input/event9:	FTSC1000:00 2808:5662 UNKNOWN
/dev/input/event10:	sof-hda-dsp HDMI/DP,pcm=3
/dev/input/event11:	sof-hda-dsp HDMI/DP,pcm=4
/dev/input/event12:	sof-hda-dsp HDMI/DP,pcm=5
/dev/input/event13:	TOPS0102:00 35CC:0104 Gesture Control
Select the device event number [0-17]: 13
Input driver version is 1.0.1
Input device ID: bus 0x18 vendor 0x35cc product 0x104 version 0x1
Input device name: "TOPS0102:00 35CC:0104 Gesture Control"
Supported events:
  Event type 0 (EV_SYN)
  Event type 1 (EV_KEY)
    Event code 35 (KEY_H)
    Event code 47 (KEY_V)
    Event code 56 (KEY_LEFTALT)
    Event code 62 (KEY_F4)
    Event code 114 (KEY_VOLUMEDOWN)
    Event code 115 (KEY_VOLUMEUP)
    Event code 125 (KEY_LEFTMETA)
    Event code 224 (KEY_BRIGHTNESSDOWN)
    Event code 225 (KEY_BRIGHTNESSUP)
Properties:
Testing ... (interrupt to exit)
Event: time 1748509887.039364, type 1 (EV_KEY), code 225 (KEY_BRIGHTNESSUP), value 1
Event: time 1748509887.039364, type 1 (EV_KEY), code 225 (KEY_BRIGHTNESSUP), value 0
Event: time 1748509887.039364, -------------- SYN_REPORT ------------
Event: time 1748509888.057592, type 1 (EV_KEY), code 224 (KEY_BRIGHTNESSDOWN), value 1
Event: time 1748509888.057592, type 1 (EV_KEY), code 224 (KEY_BRIGHTNESSDOWN), value 0
Event: time 1748509888.057592, -------------- SYN_REPORT ------------
Event: time 1748509890.115737, type 1 (EV_KEY), code 115 (KEY_VOLUMEUP), value 1
Event: time 1748509890.115737, type 1 (EV_KEY), code 115 (KEY_VOLUMEUP), value 0
Event: time 1748509890.115737, -------------- SYN_REPORT ------------
Event: time 1748509890.875741, type 1 (EV_KEY), code 114 (KEY_VOLUMEDOWN), value 1
Event: time 1748509890.875741, type 1 (EV_KEY), code 114 (KEY_VOLUMEDOWN), value 0
Event: time 1748509890.875741, -------------- SYN_REPORT ------------
Event: time 1748509894.433985, type 1 (EV_KEY), code 125 (KEY_LEFTMETA), value 1
Event: time 1748509894.433985, type 1 (EV_KEY), code 47 (KEY_V), value 1
Event: time 1748509894.433985, type 1 (EV_KEY), code 47 (KEY_V), value 0
Event: time 1748509894.433985, type 1 (EV_KEY), code 125 (KEY_LEFTMETA), value 0
Event: time 1748509894.433985, -------------- SYN_REPORT ------------
Event: time 1748509901.167982, type 1 (EV_KEY), code 125 (KEY_LEFTMETA), value 1
Event: time 1748509901.167982, type 1 (EV_KEY), code 35 (KEY_H), value 1
Event: time 1748509901.167982, type 1 (EV_KEY), code 35 (KEY_H), value 0
Event: time 1748509901.167982, type 1 (EV_KEY), code 125 (KEY_LEFTMETA), value 0
Event: time 1748509901.167982, -------------- SYN_REPORT ------------
Event: time 1748509905.651356, type 1 (EV_KEY), code 56 (KEY_LEFTALT), value 1
Event: time 1748509905.651356, type 1 (EV_KEY), code 62 (KEY_F4), value 1
Event: time 1748509905.651356, type 1 (EV_KEY), code 62 (KEY_F4), value 0
Event: time 1748509905.651356, type 1 (EV_KEY), code 56 (KEY_LEFTALT), value 0
Event: time 1748509905.651356, -------------- SYN_REPORT ------------
``
To probe your Touchpads raw output, use
```bash
sudo cat /dev/hidraw1 | hexdump -C
```
and output will show like this (for example for knocking gesture):
```
00000000  0e 06 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
```
