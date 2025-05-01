# NetworkManagerGUI

A simple and user-friendly GTK-based graphical user interface for NetworkManager on Linux systems.


## Features

- Scan for available WiFi networks
- View signal strength with visual indicators
- Connect to networks with or without password authentication
- Disconnect from connected networks
- Shows multiple access points with the same SSID
- Clear indication of currently connected network

## Requirements

- Linux operating system
- NetworkManager
- GTK+ 3.0
- gtkmm-3.0 (C++ bindings for GTK+)
- CMake (version 3.10 or higher)
- C++17 compatible compiler

## Installation

### Building from Source

1. Clone the repository:
   ```bash
   git clone https://github.com/iamSt3el/NetworkManagerGui.git
   cd NetworkManagerGui
   ```

2. Create a build directory and build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

3. Install the application:
   ```bash
   sudo make install
   ```

### Dependencies

On Debian/Ubuntu-based systems, you can install the required dependencies with:
```bash
sudo apt install build-essential cmake libgtkmm-3.0-dev network-manager
```

On Fedora/RHEL-based systems:
```bash
sudo dnf install cmake gcc-c++ gtkmm30-devel NetworkManager-devel
```

On Arch Linux:
```bash
sudo pacman -S cmake base-devel gtkmm3 networkmanager
```

## Usage

### Launch from Application Menu

After installation, you can launch NetworkManagerGUI from your desktop environment's application menu. Look for it in the "Network" or "Utilities" category.

### Launch from Terminal

You can also launch the application from the terminal:
```bash
NetworkManagerGUI
```

### Interface Guide

- **Scan Button**: Click to rescan for available networks
- **Network List**: Shows all available networks sorted by signal strength
- **Signal Strength**: Visual indicator showing the strength of each network
- **AP Count**: The number in parentheses shows how many access points are broadcasting with the same SSID
- **Connect Button**: Click to connect to a network
- **Disconnect Button**: Click to disconnect from the currently connected network

## Permissions

This application requires privileges to use NetworkManager. If you encounter permission issues, make sure your user has the appropriate permissions to control NetworkManager.

Usually, this means being in the `netdev` or `network` group:
```bash
sudo usermod -a -G netdev $USER
```
You may need to log out and log back in for the group changes to take effect.

## Troubleshooting

### Common Issues

- **"Error creating D-Bus proxy"**: Make sure NetworkManager is running
  ```bash
  systemctl status NetworkManager
  ```

- **Permission denied errors**: Check that your user has the correct permissions to use NetworkManager

- **"Connection activation failed"**: Make sure you've entered the correct password for the network

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the project
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the GPL-3.0+ License - see the LICENSE file for details.

## Acknowledgments

- Built with [GTK+](https://www.gtk.org/) and [gtkmm](https://www.gtkmm.org/)
- Uses [NetworkManager](https://wiki.gnome.org/Projects/NetworkManager) for network management
