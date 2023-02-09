# ESP32_HWSI
ESP32 to ESP32 Hard Wired Serial Interface for bidirectional string messaging.

This is an exmple for platformIO w/ Arduino.
It connects two ESP32s by hardwiring their Serial2 ports (UART_2)
in a cross wired configuration. It also cross wires their RTS and CTS for
high speed transfer flow control and it wires both devices ground pins for
a common timing ground. Simple enough. 
 * 
High speed transport is not totally explored as yet. This example sets the 
Serial2 baud_rate to 921600 - which is 8X the USB monitor_speed of 115200.
The bidirectional traffic is generated as serialized string messages of 33
characters each at a rate of four per second from both ESP32s. 

Wiring configuration: []
 * TXD2  (GPIO_NUM_17)  <——┐ cross wired
 * RXD2  (GPIO_NUM_16)  <——┘ TX <——> RX
 * RTS2  (GPIO_NUM_18)  <——┐ ... as well as 
 * CTS2  (GPIO_NUM_19)  <——┘ RTS <——> CTS

This project was to provide access to the WLAN and beyond for an ESP-Now network project.
