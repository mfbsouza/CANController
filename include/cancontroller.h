#ifndef CANCONTROLLER_H
#define CANCONTROLLER_H

#include <stdint.h>

#ifdef CAN_SPEED_1MBPS
    // bit time  = 1 us
    // tq length = 125 ns
    #define TQ_SEG1 5
    #define TQ_SEG2 2
#endif

#ifdef CAN_SPEED_800KBPS
    // bit time  = 1.25 us
    // tq length = 125 ns
    #define TQ_SEG1 6
    #define TQ_SEG2 3
#endif

#ifdef CAN_SPEED_500KBPS
    // bit time  = 2 us
    // tq length = 125 ns
    #define TQ_SEG1 12
    #define TQ_SEG2 3
#endif

#ifdef CAN_SPEED_250KBPS
    // bit time  = 4 us
    // tq length = 250 ns
    #define TQ_SEG1 12
    #define TQ_SEG2 3
#endif

#ifdef CAN_SPEED_125KBPS
    // bit time  = 8 us
    // tq length = 500 ns
    #define TQ_SEG1 12
    #define TQ_SEG2 3
#endif

#define CAN_HIGH 1
#define CAN_LOW  0
#define TRUE     1
#define FALSE    0
#define SJW      1

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

enum {
    SYNC_SEG,
    SEG_1,
    SEG_2,
    DELAY,
    ADVANCE
};

void bit_timing_fsm(void);

#endif