/*


https://github.com/mchr3k/arduino-libs-manchester



This code is based on the Atmel Corporation Manchester
Coding Basics Application Note.

http://www.atmel.com/dyn/resources/prod_documents/doc9164.pdf

Quotes from the application note:

"Manchester coding states that there will always be a transition of the message signal
at the mid-point of the data bit frame.
What occurs at the bit edges depends on the state of the previous bit frame and
does not always produce a transition. A logical '1' is defined as a mid-point transition
from low to high and a '0' is a mid-point transition from high to low.
 
We use Timing Based Manchester Decode.
In this approach we will capture the time between each transition coming from the demodulation
circuit."

Timer 2 is used with a ATMega328. Timer 1 is used for a ATtiny85 and ATtiny84

This code gives a basic data rate as 1200 bauds. In manchester encoding we send 1 0 for a data bit 0.
We send 0 1 for a data bit 1. This ensures an average over time of a fixed DC level in the TX/RX.
This is required by the ASK RF link system to ensure its correct operation.
The data rate is then 600 bits/s.
*/

#ifndef MANCHESTER_h
#define MANCHESTER_h

#define TxDefault 4 //the digital pin to use to transmit data
#define RxDefault 5 //the digital pin to use to receive data

//timer scaling factors for different transmission speeds
#define MAN_1200 0
#define MAN_2400 1
#define MAN_4800 2
#define MAN_9600 3
#define MAN_19200 4
#define MAN_38400 5
#define MAN_76800 6
#define MAN_153600 7

/*
Timer 2 in the ATMega328 and Timer 1 in a ATtiny85 is used to find the time between
each transition coming from the demodulation circuit.
Their setup is for normal count. No connections. No interupts.
For practical reasons we use power of 2 timer prescaller for sampling, 
for best timing we use pulse lenght as integer multiple of sampling speed.
We chose to sample every 16 ticks, and pulse lenght of 48 ticks 
this gives us 16000000Hz/48/256 = 1302 pulses per second (so it's not really 1200) 
At different transmission speeds or on different microcontroller frequencies, clock prescaller is adjusted 
to be compatible with those values. We allow about 50% clock speed difference both ways
allowing us to transmit even with up to 100% in clock speed difference
*/

/*
	Signal timing, we take sample every 16 clock ticks
	
	ticks:   [0]----[16]----[32]----[48]----[64]----[80]----[96]---[112]---[128]---[144]---[160]--[176]
	samples: |-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
	single:  |                [------|--------]
	double:  |                                 [-------------|----------------]
	signal:  |_______________________                         _______________________
	         |                       |_______________________|                       |_______

*/

#define MinCount 33 //pulse lower count limit on capture
#define MaxCount 65 //pulse higher count limit on capture
#define MinLongCount 66 //pulse lower count on double pulse
#define MaxLongCount 129 //pulse higher count on double pulse

#define RX_MODE_PRE 0
#define RX_MODE_SYNC 1
#define RX_MODE_DATA 2
#define RX_MODE_MSG 3
#define RX_MODE_IDLE 4

#define TimeOutDefault -1 //the timeout in msec default blocks

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include <pins_arduino.h>
#endif

class Manchester
{
  public:
    Manchester(); //the constructor
    void setTxPin(uint8_t pin); //set the arduino digital pin for transmit. default 4.
    void setRxPin(uint8_t pin); //set the arduino digital pin for receive. default 5.
    
    void setupTransmit(uint8_t pin, uint8_t SF = MAN_1200); //set up transmission
    void setupReceive(uint8_t pin, uint8_t SF = MAN_1200); //set up receiver
    void setup(uint8_t Tpin, uint8_t Rpin, uint8_t SF = MAN_1200); //set up receiver
    
    void transmit(uint16_t data); //transmit 16 bits of data
    void transmitBytes(uint8_t numBytes, uint8_t *data); // transmit a byte array
    
    uint8_t decodeMessage(uint16_t m, uint8_t &id, uint8_t &data); //decode 8 bit payload and 4 bit ID from the message, return 1 of checksum is correct, otherwise 0
    uint16_t encodeMessage(uint8_t id, uint8_t data); //encode 8 bit payload, 4 bit ID and 4 bit checksum into 16 bit
    
    //wrappers for global functions
    void beginReceive(void);
    uint8_t receiveComplete(void);
    uint16_t getMessage(void);
    void stopReceive(void);
    
  private:
    void sendZero(void);
    void sendOne(void);
    uint8_t TxPin;
    uint8_t speedFactor;
    uint16_t delay1;
    uint16_t delay2;
};//end of class Manchester

// Cant really do this as a real C++ class, since we need to have
// an ISR
extern "C"
{
    //set the arduino digital pin for receive. default 4.
    extern void MANRX_SetRxPin(uint8_t pin);
    
    //begin the timer used to receive data
    extern void MANRX_SetupReceive(uint8_t speedFactor = MAN_1200);
    
    // begin receiving 16 bits
    extern void MANRX_BeginReceive(void);
    
    // begin receiving a byte array
    extern void MANRX_BeginReceiveBytes(uint8_t maxBytes, uint8_t *data);
    
    // true if a complete message is ready
    extern uint8_t MANRX_ReceiveComplete(void);
    
    // fetch the received message
    extern uint16_t MANRX_GetMessage(void);
    
    // fetch the received message
    extern void MANRX_GetMessageBytes(uint8_t *rcvdBytes, uint8_t **data);
    
    // stop receiving data
    extern void MANRX_StopReceive(void);
}

extern Manchester man;

#endif