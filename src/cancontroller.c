#include <cancontroller.h>

uint8_t hard_sync   = FALSE, soft_sync    = FALSE;
uint8_t write_point = FALSE, sample_point = FALSE;

uint8_t state       = 0, tq_count     = 0;
uint8_t phase_error = 0;

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