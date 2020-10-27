#ifndef CANCONTROLLER_H
#define CANCONTROLLER_H

#include <stdint.h>
#define __AVR_ATmega328P__
#include <avr/io.h>
#include <avr/interrupt.h>

#if defined (CAN_SPEED_1MBPS)
    // bit time  = 1 us
    #define TQ_LENGTH 125 // 125 ns
    #define TQ_SEG1   5
    #define TQ_SEG2   2
#elif defined (CAN_SPEED_800KBPS)
    // bit time  = 1.25 us
    #define TQ_LENGTH 125 // 125 ns
    #define TQ_SEG1 6
    #define TQ_SEG2 3
#elif defined (CAN_SPEED_500KBPS)
    // bit time  = 2 us
    #define TQ_LENGTH 125 // 125 ns
    #define TQ_SEG1 12
    #define TQ_SEG2 3
#elif defined (CAN_SPEED_250KBPS)
    // bit time  = 4 us
    #define TQ_LENGTH 250 // 250 ns
    #define TQ_SEG1 12
    #define TQ_SEG2 3
#else /* Default Speed: 125KBPS */
    // bit time  = 8 us
    #define TQ_LENGTH 500 // 500 ns
    #define TQ_SEG1 12
    #define TQ_SEG2 3
#endif

#ifndef F_CPU
    #define F_CPU 16000000UL
#endif

#define CAN_HIGH 1
#define CAN_LOW  0
#define TRUE     1
#define FALSE    0
#define SJW      1


/* X: time in nanoseconds */
#define ticks(X) ((X * (F_CPU/1000000000)))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

enum {
    SYNC_SEG,
    SEG_1,
    SEG_2,
    DELAY,
    ADVANCE
};


/* functions */
void can_init(void);
void edge_detector(void);
void bit_timing_fsm(void);

#endif