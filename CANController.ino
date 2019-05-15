#include <TimerOne.h>

/* ----------- PARAMETERS ----------- */

// #define TQ 1000000 // in microseconds
#define TQ 250000
#define TQ_SEG1 1
#define TQ_SEG2 2
#define SJW 2

#define CANRX 9
#define CANTX 8
#define PSBUT 12
#define LED 13

struct Base_frame {
  byte start_of_frame;
  unsigned int id;
  byte rtr;
  byte ide;
  byte r0;
  byte dlc;
  unsigned long long data;
  unsigned int crc;
  byte crc_d;
  byte ack;
  byte ack_d;
  byte end_of_frame;
};

bool once = true;

// testing frames

Base_frame a {
  .start_of_frame = 0,
  .id = 1365,
  .rtr = 0,  // data frame
  .ide = 0,  // base frame
  .r0 = 1,
  .dlc = 1,  // 1 byte of data
  .data = 153, // data
  .crc = 22937,
  .crc_d = 1, // recessive
  .ack = 0,
  .ack_d = 1, // recessive
  .end_of_frame = 0 // 7 bits recessive;
};

void setup() {
  Serial.begin(9600);
  pinMode(CANTX, OUTPUT);
  pinMode(CANRX, INPUT);
  pinMode(PSBUT, INPUT);
  pinMode(LED, OUTPUT);
  Timer1.initialize(TQ);
  Timer1.attachInterrupt(always_ff);
}

void loop() {
  edgeDet();
  bitSampler();
  BFalways_ff();
  if(digitalRead(PSBUT) == HIGH && once){
    once = false;
    send_BaseFrame(&a);
  }
  
}
