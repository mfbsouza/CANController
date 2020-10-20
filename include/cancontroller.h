#ifndef CANCONTROLLER_H
#define CANCONTROLLER_H

#include <stdint.h>

#define CAN_HIGH 1
#define CAN_LOW  0
#define TQ_SEG1  10
#define TQ_SEG2  10
#define SJW      10
#define TRUE     1
#define FALSE    0

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