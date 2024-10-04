## Information regards this codebase

Due to the lengthy development process and numerous necessary workarounds, the final codebase feels like a patchwork. Ideally, it should be rewritten from scratch. However, this would require extensive time for redevelopment and retesting. Since there is no significant benefit to doing so, the code will continue to be used in its current form.

## For future reference

Notes reflecting design decisions for future reference.

### EspNow

To use EspNow with active WiFi and web functionality, the WiFi module has to be in WIFI_STA_AP mode and all devices must use the same WiFi channel.

### Arduino Core

The Arduino Core library could be completely replaced by the EspIDF framework. Essentially, it's only used to simplify the project. However, this can sometimes lead to issues as various functionalities are not available.

### Socket

The communication for the web interface is handled via sockets. Only with this method was the communication fast enough to implement a proper press, hold, and release functionality for buttons.

### Web

The interface could be replaced with a proper React framework.
