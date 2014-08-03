HID reports over UART to USB Converter
========================================
This takes 'raw' HID reports in the format described for Adafruit's Bluefruit or Roving Networks' RN-42 HID Bluetooth module.
Target MCU is the ATmega 16u2, which is the USB chip on the Arduino UNO.  The purpose of this module is to make it possible use
a microcontroller without USB hardware (such as the ATmega 328p on the UNO) to run the TMK firmware, but offload the USB work to
a dedicated USB chip.  This could be useful to allow switching between Bluetooth and USB for example.


Hardware
--------
Connect RX, TX and GND to UART pin of AVR. Note that you may need line driver/level shifter like MAX232 to interface high voltage of RS-232C.



Build Firmware
--------------
Configure UART setting and Just use 'make'

    $ cd serial_usb
    $ make

Then, load the binary to MCU with your favorite programmer.



Limitation
----------

