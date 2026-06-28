# nl-ease
# TESTING BRANCH
A tool to easily set the night light. Simple tool written in C and the Enlightenment Foundation Libraries. It allows you to choose the color intensity thanks to **xrandr**, set a start time, end time and save the settings. \
(nl = "night light")

Designed for Bodhi Linux 7 but may also work on other distros with EFL development package installed.\
Supported languages: English, French, Italian, Polish and Portuguese

<img width="308" height="316" alt="shot-2026-06-28_16-39-57" src="https://github.com/user-attachments/assets/bf7c9c7d-dcde-4609-8b33-53235cabc7da" />

## Requirements:

nl-ease requires an installation of EFL development package.  (Debian 13 based distros may also need: `liblua5.1-0-dev`)

<ins>***Example EFL development package installs (Bodhi Linux 7):***</ins>
```
sudo apt update
sudo apt install libefl-dev
```
### Installation:

- **Compile:**  `make`

- **Install:**  `sudo make install`

- **Uninstall:**  `sudo make uninstall`

 ### Note: 

Moksha users can also create a personal application launcher that runs the terminal command `nl-ease --daemon` and set it as a startup application. This automates the loading of the configuration file and eliminates the need to manually launch the daemon every time the system starts.
