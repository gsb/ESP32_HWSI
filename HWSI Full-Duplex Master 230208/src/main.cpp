
//-----------------------------------------------------------------------------
/*
 *  This example connects two ESP32s by hardwiring their Serial2 ports (Uart_2)
 *  in a cross wired configuration. It also cross wires their RTS and CTS for
 *  high speed transfer flow control and it wires both devices ground pins for
 *  a common timing ground. Simple enough. 
 * 
 *  High speed transport is not totally explored as yet. This example sets the 
 *  Serial2 baud_rate to 921600 - which is 8X the USB monitor_speed of 115200.
 *  The bydirectional traffic is generated as serialized string messages of 33
 *  characters each at a rate of four per second from both ESP32s. 
 * 
 *    Wiring configuration: []
 *      TXD2  (GPIO_NUM_17)  <——┐ cross wired
 *      RXD2  (GPIO_NUM_16)  <——┘ TX <——> RX
 *      RTS2  (GPIO_NUM_18)  <——┐ ... as well as 
 *      CTS2  (GPIO_NUM_19)  <——┘ RTS <——> CTS
 */
//-----------------------------------------------------------------------------

#include <Arduino.h>
#include "driver/uart.h"
#include <queue>

//-- Global Variables, Definitions, and the like

#define HWSI  Serial2  // using standard Uatr_2 pin settings

#define TXD2  (GPIO_NUM_17) // (UART_PIN_NO_CHANGE)  <——┐ cross wired 
#define RXD2  (GPIO_NUM_16) // (UART_PIN_NO_CHANGE)  <——┘ TX <——> RX
#define RTS2  (GPIO_NUM_18) // (UART_PIN_NO_CHANGE)  <——┐ ... as well as 
#define CTS2  (GPIO_NUM_19) // (UART_PIN_NO_CHANGE)  <——┘ RTS <——> CTS

// Two FIFO queues for activities seperation, as in...
//  - build, queue and then send messages in sequence (FIFO hwsique), and
//  - read, queue and then process incoming messages in sequence (FIFO pending)
std::queue<String>hwsique; //FIFO queue of messages awaiting HWSI transport
std::queue<String>pending; //FIFO queue of message arrivals awaiting processing

String espName = ESP_NAME; // cosmedic name for monitoring

char incoming[MAX_MSG_SIZE+1]; // Incoming message character buffer


//-- Arduino Core Methods - Setup ---------------------------------------------
void setup()
{
  Serial.begin(SERIAL_BAUD);

  HWSI.setRxBufferSize(MAX_MSG_SIZE*4);
  HWSI.setTxBufferSize(MAX_MSG_SIZE*4);
  HWSI.begin(HWSI_BAUD);
  uart_set_pin(UART_NUM_2, RXD2, TXD2, RTS2 , CTS2);
  uart_set_hw_flow_ctrl(UART_NUM_2, UART_HW_FLOWCTRL_CTS_RTS, 32); //64);
  uart_set_mode(UART_NUM_2, UART_MODE_UART);
  HWSI.flush(); // clean out the HWSI connection before looping begins
  
  Serial.println("Setup complete!\n");
}

//-- Arduino Core Methods - Loop ---------------------------------------------
void loop()
{
  //-- Periodic Message Generator, on both ESP32s devices (for testing only)
  static unsigned long last = micros();  // last time checked in micros
  static unsigned int interval = 250000; // interval delay in micros (µs) 4/sec
  static unsigned int msgID = 0; // counter for tracking message by msgID
  if (micros() - last >= interval) // trigger now, i.e. do it again now
  {
    last = micros() - (micros() - (last + interval));
    #if (BOARD_ID == 100) // is the gateway esp
      hwsique.push((String("/777/esp101/esp100/@2per-sec/")+(msgID++)));
    #elif (BOARD_ID == 101) // is the hub esp
      hwsique.push((String("/888/esp100/esp101/@2per-sec/")+(msgID++)));
    #endif
  }

  // ...process, if 'availableForWrite', the first queued outgoing message
  if (hwsique.size()) // outgoing messages are available to transfer, so
  {
    String msg = hwsique.front(); // try now by getting first queued message, 
    if (HWSI.availableForWrite() >= msg.length()) // based on its length,
    {
      hwsique.pop(); // if enough space, remove it from hwsique and send it now
      HWSI.print(msg.c_str()); // as outgoing to the other ESP32 via HWSI and
      HWSI.flush(true); // flush to ensure clean transport before next message
      Serial.println(espName +" >> outgoing: "+ msg); // for testing only...
    }
  }

  // ...receive incoming messages
  int n = HWSI.available(); // Temporary variable for how much data is availabe
  if (n) // if HWSI data is available, read it in as a message and queue it
  {
    if ((n=HWSI.read((char*)&incoming, min(n, MAX_MSG_SIZE))))
    {
      incoming[n] ='\0'; // properly terminate incoming message string
      String msg = incoming; 
      pending.push(msg); // push onto pending for processing later
      Serial.println(espName+" << incoming: "+msg); // for testing only...
    }
  } 

  // ...process queued incoming messages awaiting local action
  if (pending.size()) //...if no messages are queued, skip
  {
    String msg = pending.front(); // Else, get first queued message, remove it
    pending.pop();                // from queue and process it below

    // local message request processing code goes here but for now, simpy print
    Serial.println(espName +"  processing: "+ msg.c_str());
  }
}

//-----------------------------------------------------------------------------
