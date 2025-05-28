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

## 3: Deploy service

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
```

Your gesture service should now be running as a proper systemd service with logging and automatic startup
