#include <cancontroller.h>

static uint8_t hard_sync   = FALSE, soft_sync    = FALSE;
static uint8_t write_point = FALSE, sample_point = FALSE;
static uint8_t state       = 0,     bus_idle     = FALSE;
static uint8_t tq_count    = 0,     phase_error  = 0;

/* Interrupt Service Routines */

ISR (TIMER0_COMPA_vect) {
    bit_timing_fsm();
}

ISR (INT0_vect) {
    edge_detector();
}

/* Functions */

void can_init(void) {
    // INT0/PD2 as CAN RX
    MCUCR  |= (1 << PUD);
    DDRD   |= (1 << PD2);
    EICRA  |= (1 << ISC01) | (1 << ISC00);
    EIMSK  |= (1 << INT0);

    // Bit Timing Interrupt
    TCCR0B |= (1 << CS00);
    TCCR0A |= (1 << WGM01);
    TCNT0   = 0x00;
    OCR0A   = (uint8_t) ticks(TQ_LENGTH);
    TIMSK0 |= (1 << OCIE0A);

    // Enable CPU Interrupts
    sei();
}

void edge_detector (void) {
    if(bus_idle) hard_sync = TRUE;
    else soft_sync = TRUE;
}

void bit_timing_fsm (void) {

    if (hard_sync) {
        hard_sync = FALSE;
        tq_count  = 0;
        state     = SYNC_SEG;
    }

    switch (state) {
        case SYNC_SEG:
            write_point = TRUE;
            tq_count    = 0;
            state       = SEG_1;
            break;
        
        case SEG_1:
            tq_count++;
            if (write_point) write_point = FALSE;

            if (soft_sync) {
                soft_sync   = FALSE;
                phase_error = TQ_SEG1 + MIN(SJW, tq_count);
                state       = DELAY;
            }

            else if (tq_count == TQ_SEG1) {
                sample_point = TRUE;
                tq_count     = 0;
                state        = SEG_2;
            }
            break;
        
        case SEG_2:
            tq_count++;
            if (sample_point) sample_point = FALSE;

            if (soft_sync) {
                soft_sync   = FALSE;
                phase_error = TQ_SEG2 - MIN(SJW, tq_count);
                
                if (tq_count >= phase_error) {
                    tq_count = 0;
                    state    = SEG_1;
                }
                else state = ADVANCE;
            }

            else if (tq_count == TQ_SEG2) state = SYNC_SEG;
            break;

        case DELAY:
            tq_count++;
            if (tq_count == phase_error) {
                sample_point = TRUE;
                tq_count     = 0;
                state        = SEG_2;
            }
            break;
        
        case ADVANCE:
            tq_count++;
            if (tq_count == phase_error) {
                tq_count = 0;
                state    = SEG_1;
            }
            break;
    }
}