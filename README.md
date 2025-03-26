Bangle.js Minimal C Example Code
=====================================

This is a minimal example of running your own barebones C code [on Bangle.js 2](https://www.espruino.com/Bangle.js2), with 
support for Bluetooth, LCD, Button and Touchscreen.

Example code is based on `nrf5x_17/examples/ble_peripheral/ble_app_uart/main.c` (for BLE UART capability)

See https://www.espruino.com/Bangle.js2+Technical for full details of pinouts and hardware.

Building
--------

* Download `nRF5 SDK` version 17 from https://www.nordicsemi.com/Products/Development-software/nRF5-SDK installed in the `nrf5x_17` directory
* Ensure you have `arm-none-eabi-gcc` installed and `nrf5x_17/components/toolchain/gcc/Makefile.posix` is set up to point to it
* Get an nRF52DK wired up to the Bangle.js USB charge cable as shown in https://www.espruino.com/Bangle.js2+Technical#swd (while in theory
you can develop wirelessly via DFU updates, it's no fun at all and you're liable to brick your device)

```
# make the software
make
# write the softdevice (only need to do this once)
make flash_softdevice
# write the actual firmware
make flash
```


Implemented
-------------

* Bluetooth UART
* Button
* Kick Watchdog timer once a second while button is pressed (long press still reboots to bootloader)
* Charging detect
* Backlight
* LCD screen (simple text, fillrect)
* Touchscreen



Not Implemented
----------------

* Accelerometer
* Magnetometer
* HRM


Notes
------

* We use software I2C for touch, and software SPI for the LCD screen just for simplicity (hardware would allow background updates to the screen)
* The LCD is using 4 bit mode (3 bit mode or black and white is possible - but again, simplicity)
* While on, the touchscreen draws ~1.5mA, and the backlight draws ~60mA
