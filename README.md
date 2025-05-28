# honor-magicbook-art-touchpad-gestures

## 1: Clone repo
```bash
git clone https://github.com/MadhiasM/honor-magicbook-art-touchpad-gestures
```

## 2: Compile service

```bash
# Compile service
gcc -o gesture-daemon gesture-daemon.c

# Install to system location
sudo cp gesture-daemon /usr/local/bin/
sudo chmod +x /usr/local/bin/gesture-daemon
```

## 3: Deploy service

```bash
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
