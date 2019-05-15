/* BIT TIMING UNIT */

bool busIdle = true, can_rx = 1;               // inputs
bool sample_point = false, write_point = false;     // outputs
bool lsp = false;

// FSM Variables 
byte tq_count = 0, seg1_time = 0, seg2_cnt = 0;
byte state = 0, next_state = 0, PhaseError = 0;
bool hard_sync = false, soft_sync = false;

// Edge & Sample Bit
byte sample_bit = 1;
bool edge_on = true;
byte last_sample_bit = 1;

/* enums */

enum states {
  SYNC_SEG,
  SEG_1,
  SEG_2,
  DELAY,
  ADVANCE
};

/* functions */

bool negedge(byte current, byte *last){
  if(current != (*last) && current == LOW){
    (*last) = current;
    return true;
  }
  (*last) = current;
  return false;
}

void edgeDet(){
  byte current = !digitalRead(CANRX);
  if(negedge(current, &last_sample_bit)){
    if(busIdle){
      hard_sync = true;
    } else {
      soft_sync = true;
      //Serial.print("ss");
    }
  }
}

void bitSampler(){
  if(posedge(sample_point,&lsp)){
    sample_bit = !digitalRead(CANRX);
  }
}

void always_ff() {
  digitalWrite(LED, LOW);
  //edgeDet(); // check syncs
  FSM_comb();     // does bit sampling + write_point & sample_point
  state = next_state;
  tq_count++;
}

void FSM_comb() {

  if (hard_sync) {
    hard_sync = false;
    tq_count = 0;
    state = SEG_1;
  }

  switch(state) {
    
    case SYNC_SEG:
      write_point = true;
      tq_count = 0;
      next_state = SEG_1;
      break;

    case SEG_1:
      write_point = false;

      if (soft_sync) {
        soft_sync = false;
        PhaseError = tq_count;
        next_state = DELAY;
      }
      else if (tq_count < TQ_SEG1) {
        next_state = SEG_1;
      }
      else {
        //sample_bit = !digitalRead(CANRX);
        sample_point = true;
        digitalWrite(LED, HIGH);
        seg1_time = tq_count;
        next_state = SEG_2;
      }
      break;

    case SEG_2:
      seg2_cnt = tq_count - seg1_time;
      sample_point = false;
      
      if (soft_sync) {
        soft_sync = false;
        PhaseError = seg2_cnt;
        
        if (seg2_cnt >= (TQ_SEG2 - min(SJW, PhaseError))) {
          tq_count = 0;
          next_state = SEG_1;
        } else {
          next_state = ADVANCE;
        }
      }
      else if (seg2_cnt < TQ_SEG2) {
        next_state = SEG_2;
      }
      else {
        next_state = SYNC_SEG;
      }
      break;
      
    case DELAY:
      if (tq_count < (TQ_SEG1 + min(SJW, PhaseError))) {
        next_state = DELAY;
      } else {
        //sample_bit = can_rx;
        //last_sample_bit = sample_bit;
        //sample_bit = !digitalRead(CANRX);
        //Serial.print(sample_bit);
        sample_point = true;
        digitalWrite(LED, HIGH);
        seg1_time = tq_count;
        next_state = SEG_2;
      }
      break;
    case ADVANCE:
      seg2_cnt = tq_count - seg1_time;
      if (seg2_cnt < (TQ_SEG2 - min(SJW, PhaseError))) {
        next_state = ADVANCE;
      } else {
        tq_count = 0;
        next_state = SEG_1;
      }
      break;
  }
}
