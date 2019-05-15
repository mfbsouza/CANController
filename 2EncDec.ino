/* ENCODER/DECODER UNIT */

#define BUFFER_SIZE 2

struct Extended_frame {
  byte start_of_frame;
  unsigned int id;
  byte srr;
  byte ide;
  long id_b;
  byte rtr;
  byte r1r0;
  byte dlc;
  unsigned long long data;
  unsigned int crc;
  byte crc_d;
  byte ack;
  byte ack_d;
  byte end_of_frame;
};

/* build frame states */
enum bf_states {
  Idle,       Sof,  ID,            RTRorSRR, IDE,
  R0,         ID_B, RTR,           R1R0,     DLC,
  Data_field, CRC,  CRC_d,         ACK,      ACK_d,
  Eof,        IFS,  ErrorOverload, WaitBit,  Error_d
};

/* FLAGS */
bool data_f = false,      remote_f = false;
bool base_f = false,      extended_f = false;  
bool error_f = false,     arbitration = false;
bool bitstuff_on = false, Writing = false;
bool ack_sender = false,  ack_check = false;

/* bit stuffing variables */
bool bitstuff_out = 1;
byte bitstuff_in  = 1;

// software clocks
bool last_sp = false;
bool last_wp = false;
bool fake_wp = false, fake_sp = false;
bool last_fwp = false, last_fsp = false;

/* Build Frame Variables */
byte bf_state     = Idle;
byte bf_nextstate = Idle;

// recv buffers
struct Base_frame     recv_b_buffer;
struct Extended_frame recv_e_buffer;
struct Extended_frame tmp_buff;

// auxliar variables
int dlc = 0;

// time counters;
int id_time    = 10;
int id_b_time  = 17;
int r1r0_time  = 1;
int dlc_time   = 3;
int data_time  = 0;
int crc_time   = 14;
int eof_time   = 6;
int ifs_time   = 2;
int error_time = 5;
int er_d_time  = 6;

// sender variables
int  send_b_index = -1;
int  send_e_index = -1;
byte dlc_backup   = 0;

struct Base_frame     *send_b_buffer[BUFFER_SIZE];
struct Extended_frame *send_e_buffer[BUFFER_SIZE];


/* functions */

bool send_BaseFrame(struct Base_frame *frame){
  if(send_b_index < (BUFFER_SIZE - 1)){
    send_b_index++;
    send_b_buffer[send_b_index] = frame;
    return true; 
  } else {
    return false;
  }
}

bool send_ExtFrame(struct Extended_frame *frame){
  if(send_e_index < (BUFFER_SIZE - 1)){
    send_e_index++;
    send_e_buffer[send_e_index] = frame;
    return true; 
  } else {
    return false;
  }
}

void BuildFrame(){

  switch(bf_state) {

    if(error_f){
      error_f = false;
      bf_state = ErrorOverload;
      Writing     = true;
      base_f      = false;
      extended_f  = false;
      bitstuff_on = false;
    }

    case ErrorOverload:
      bitstuff_in = 0;
      error_time--;
      if(error_time == -1){
        bf_nextstate = WaitBit;
        error_time = 5;
        Writing = false;
      } else {
        bf_nextstate = ErrorOverload;
      }
      break;

    case WaitBit:
      if(bitstuff_out == 0){
        bf_nextstate = WaitBit;
      } else {
        bf_nextstate = Error_d;
        Writing = true;
      }
      break;

    case Error_d:
      bitstuff_in = 1;
      er_d_time--;
      if(er_d_time == -1){
        bf_nextstate = IFS;
        er_d_time = 6;
      } else {
        bf_nextstate = Error_d;
      }
      break;
    
    case Idle:
    
      bf_nextstate = Idle;
      busIdle = true;

      
      if (!bitstuff_out) {
        tmp_buff.start_of_frame = bitstuff_out;
        tmp_buff.id = 0; // cleaning mem trash
        Serial.print("SOF: ");
        Serial.print(bitstuff_out);
        bf_nextstate = ID;
        Serial.print(" ID: ");
        busIdle      = false;
        Writing      = false;
        bitstuff_on  = true;
        arbitration  = true;

      // if have something to send
      } else if (send_b_index >= 0) {
        base_f = true;
        edge_on = false;
        bf_nextstate = Sof;
      } else if (send_e_index >= 0) {
        extended_f = true;
        edge_on = false;
        bf_nextstate = Sof;
      }
      break;

    case Sof:
      if (!bitstuff_out) {
        tmp_buff.start_of_frame = bitstuff_out;
        tmp_buff.id = 0; // cleaning mem trash
        
        bf_nextstate = ID;
        busIdle = false;
        Writing      = false;
        bitstuff_on  = true;
        arbitration  = true;

      // writing 
      } else {
        ifs_time = 3;
        Writing = true;
        bitstuff_on = true;
        arbitration = true;
        busIdle = false;
        if(base_f){
          bitstuff_in = send_b_buffer[send_b_index]->start_of_frame;
        } else {
          bitstuff_in = send_e_buffer[send_e_index]->start_of_frame;
        }
        bf_nextstate = ID;
      }
      break;

    case ID:
    
      if (!Writing) {
        bitWrite(tmp_buff.id, id_time, bitstuff_out);
        Serial.print(bitstuff_out);
      } else {
        if(base_f){
          bitstuff_in = bitRead(send_b_buffer[send_b_index]->id, id_time);
        } else {
          bitstuff_in = bitRead(send_e_buffer[send_e_index]->id, id_time);
        }
      }
      id_time--;  
      if(id_time == -1) {
         bf_nextstate = RTRorSRR;
         id_time = 10;
      } else {
        bf_nextstate = ID;
      }
      break;

    case RTRorSRR:
    
      if (!Writing) {
        Serial.print(" RTRorSRR: ");
        tmp_buff.srr = bitstuff_out;
        Serial.print(bitstuff_out);
        Serial.print(" IDE: ");
      } else if(base_f) {
        bitstuff_in = send_b_buffer[send_b_index]->rtr;
        if(bitstuff_in == 0) {
          data_f = true;
        } else {
          remote_f = true;
        }
      } else {
        bitstuff_in = send_e_buffer[send_e_index]->srr;
      }
      bf_nextstate = IDE;
      break;

    case IDE:
    
      if(!Writing) {
        tmp_buff.ide = bitstuff_out;
        Serial.print(bitstuff_out);
        if(bitstuff_out) {
          extended_f = true;
          base_f = false;
          bf_nextstate = ID_B;
          Serial.print(" ID B: ");
        } else {
          base_f = true;
          extended_f = false;
          arbitration = false;
          bf_nextstate = R0;
          Serial.print(" R0: ");
        }
      } else {
        if(base_f){
          bitstuff_in = send_b_buffer[send_b_index]->ide;
          bf_nextstate = R0;
          arbitration = false;
          dlc_backup = send_b_buffer[send_b_index]->dlc;
        } else {
          bitstuff_in = send_e_buffer[send_e_index]->ide;
          bf_nextstate = ID_B;
          dlc_backup = send_e_buffer[send_e_index]->dlc;
        }
      }
      break;

    case R0:
      if(!Writing) {
        tmp_buff.r1r0 = bitstuff_out;
        Serial.print(bitstuff_out);
        Serial.print(" DLC ");
        if(!tmp_buff.srr){
          data_f = true;
        } else {
          remote_f = true;
        }
      } else {
        bitstuff_in = send_b_buffer[send_b_index]->r0;
      }
      bf_nextstate = DLC;
      //Serial.print(" DLC: ");
      break;

    case ID_B:
      if(!Writing) {
        bitWrite(tmp_buff.id_b, id_b_time, bitstuff_out);
        Serial.print(bitstuff_out);    
      } else {
        bitstuff_in = bitRead(send_e_buffer[send_e_index]->id_b, id_b_time);
      }
      id_b_time--;
      if(id_b_time == -1) {
        bf_nextstate = RTR;
        id_b_time == 17;
      } else {
        bf_nextstate = ID_B;
      }
      break;

    case RTR:
      if(!Writing) {
        Serial.print(" RTR: ");
        tmp_buff.rtr = bitstuff_out;
        Serial.print(bitstuff_out);
        if(!bitstuff_out) {
          data_f = true;
        } else {
          remote_f = true;
        }
      } else {
        bitstuff_in = send_e_buffer[send_e_index]->rtr;
        if(bitstuff_in == 0){
          data_f = true;
        } else {
          remote_f = true;
        }
      }
      bf_nextstate = R1R0;
      Serial.print(" R1R0: ");
      arbitration = false;
      break;

    case R1R0:
      if(!Writing) {
        bitWrite(tmp_buff.r1r0, r1r0_time, bitstuff_out);
        Serial.print(bitstuff_out);
      } else {
        bitstuff_in = bitRead(send_e_buffer[send_e_index]->r1r0, r1r0_time);
      }
      r1r0_time--;
        
      if(r1r0_time == -1) {
        bf_nextstate = DLC;
        Serial.print(" DLC: ");
        r1r0_time = 1;
      } else {
        bf_nextstate = R1R0;
      }
      break;

    case DLC:
      if(!Writing) {
        bitWrite(tmp_buff.dlc, dlc_time, bitstuff_out);
        Serial.print(bitstuff_out);
      } else {
        if(base_f){
          bitstuff_in = bitRead(send_b_buffer[send_b_index]->dlc, dlc_time);
        } else {
          bitstuff_in = bitRead(send_e_buffer[send_e_index]->dlc, dlc_time);
        }
      }
      dlc_time--;
      
      if(dlc_time == -1) {
        if(!Writing){
          dlc_backup = tmp_buff.dlc;
        }
        dlc_time = 3;
        if(remote_f || dlc_backup == 0) {
          bf_nextstate = CRC;
          Serial.print(" CRC: ");
          bitstuff_on = false;
        } else {
          if(dlc_backup > 8){
            dlc_backup = 8;
          }
          data_time = ((dlc_backup * 8) - 1);
          bf_nextstate = Data_field;
          Serial.print(" DATA: ");
        }
      } else {
        bf_nextstate = DLC;
      }
      break;

    case Data_field:
      if(!Writing) {
        bitWrite(tmp_buff.data, data_time, bitstuff_out);
        Serial.print(bitstuff_out);
      } else {
        if(base_f) {
          bitstuff_in = bitRead(send_b_buffer[send_b_index]->data, data_time);
        } else {
          bitstuff_in = bitRead(send_e_buffer[send_e_index]->data, data_time);
        }
      }
      data_time--;
        
      if(data_time == -1){
        bf_nextstate = CRC;
        Serial.print(" CRC: ");
        bitstuff_on = false;
      } else {
        bf_nextstate = Data_field;
      }
      break;

    case CRC:
      if(!Writing) {
        bitWrite(tmp_buff.crc, crc_time, bitstuff_out);
        Serial.print(bitstuff_out);
      } else {
        if(base_f){
          bitstuff_in = bitRead(send_b_buffer[send_b_index]->crc, crc_time);
        } else {
          bitstuff_in = bitRead(send_e_buffer[send_e_index]->crc, crc_time);
        }
      }
      crc_time--;
        
      if(crc_time == -1){
        crc_time = 14;
        bf_nextstate = CRC_d;
      } else {
        bf_nextstate = CRC;
      }
      break;

    case CRC_d:
      if(!Writing) {
        Serial.print(" CRC_d: ");
        tmp_buff.crc_d = bitstuff_out;
        Serial.print(bitstuff_out);
      } else {
        if(base_f){
          bitstuff_in = send_b_buffer[send_b_index]->crc_d;
        } else {
          bitstuff_in = send_e_buffer[send_e_index]->crc_d;
        }
      }
      bf_nextstate = ACK;
      break;

    case ACK:
      if(!Writing) {
        Serial.print(" ACK: ");
        tmp_buff.ack = bitstuff_out;
        Serial.print(bitstuff_out);
        
      } else {
        if(base_f){
          bitstuff_in = send_b_buffer[send_b_index]->ack;
        } else {
          bitstuff_in = send_e_buffer[send_e_index]->ack;
        }
      }
      bf_nextstate = ACK_d;
      //Serial.print(" ACK_d: ");
      break;

    case ACK_d:
      if(!Writing) {
        Serial.print(" ACK_d: ");
        tmp_buff.ack_d = bitstuff_out;
        Serial.print(bitstuff_out);
        Serial.print(" EOF: ");
      } else {
        if(base_f){
          bitstuff_in = send_b_buffer[send_b_index]->ack_d;
        } else{
          bitstuff_in = send_e_buffer[send_e_index]->ack_d;
        }
      }
      bf_nextstate = Eof;
      break;

    case Eof:
      if(!Writing){
        bitWrite(tmp_buff.end_of_frame, eof_time, bitstuff_out);
        Serial.print(bitstuff_out);
      } else {
        if(base_f){
          bitstuff_in = bitRead(send_b_buffer[send_b_index]->end_of_frame, eof_time);
        } else {
          bitstuff_in = bitRead(send_e_buffer[send_e_index]->end_of_frame, eof_time);
        }
      }
      eof_time--;
        
      if(eof_time == -1){
        eof_time = 6;
        bf_nextstate = IFS;
        Serial.print(" IFS: ");
      } else {
        bf_nextstate = Eof;
      }
      break;

    case IFS:
      if(!Writing){
        Serial.print(bitstuff_out);
      } else {
        bitstuff_in = 1;
      }
      ifs_time--;
      if(ifs_time == -1){
        ifs_time = 2;
        bf_nextstate = Idle;
        //Serial.print(" Idle: ");
        if(Writing) {
          Writing = false;
          if(base_f){
            send_b_buffer[send_b_index] = NULL;
            send_b_index--;
          } else if(extended_f) {
            send_e_buffer[send_e_index] = NULL;
            send_e_index--;
          }
          if(send_b_index >= 0){
            data_f = true;
            extended_f = false;
            bf_nextstate = Sof;
          }else if(send_e_index >= 0){
            data_f = false;
            extended_f = true;
            bf_nextstate = Sof;
          }
        } else if(base_f) {
          recv_b_buffer.start_of_frame = tmp_buff.start_of_frame;
          recv_b_buffer.id = tmp_buff.id;
          recv_b_buffer.rtr = tmp_buff.srr;
          recv_b_buffer.ide = tmp_buff.ide;
          recv_b_buffer.r0 = tmp_buff.r1r0;
          recv_b_buffer.dlc = tmp_buff.dlc;
          recv_b_buffer.data = tmp_buff.data;
          recv_b_buffer.crc = tmp_buff.crc;
          recv_b_buffer.crc_d = tmp_buff.crc_d;
          recv_b_buffer.ack = tmp_buff.ack;
          recv_b_buffer.ack_d = tmp_buff.ack_d;
          recv_b_buffer.end_of_frame = tmp_buff.end_of_frame;
          // reset flags
          data_f = base_f = error_f = bitstuff_on = false;
          remote_f = extended_f = arbitration = Writing = false;
          edge_on = true;
        } else {
          recv_e_buffer = tmp_buff;
          // reset flags
          data_f = base_f = error_f = bitstuff_on = false;
          remote_f = extended_f = arbitration = Writing = false;
          edge_on = true;
        }
      } else {
        bf_nextstate = IFS;
      }
      break;
  }
}

bool posedge(bool current, bool *last) {
  
  if(current != (*last) && current) {
    (*last) = current;
    return true;
  }
  (*last) = current;
  return false;
}

void BFalways_ff() {
  
  if (Writing) {
    if(posedge(write_point, &last_wp)){
      //Serial.print(bf_state);
      //Serial.print(": ");
      //Serial.print(bitstuff_in); // sender
      //Serial.print(" ");
      BitStuffing();
      /*
      if(bf_state == ACK){
        ack_check = true;
        ack_sender = true;
        Writing = false;
      }
      */
      if(posedge(fake_wp, &last_fwp) && !ack_sender){
        bf_state = bf_nextstate;
        BuildFrame();
        fake_wp = false;
        last_fwp = false;
      } else if(ack_sender && !ack_check){
        ack_sender = false;
        Writing = false;
      }
    }
  }
  else {
    if(posedge(sample_point, &last_sp)){
      bitstuff_in = HIGH; // safe
      BitStuffing();
      //Serial.print(bitstuff_out); //recv
      if(posedge(fake_sp, &last_fsp) && !ack_check){
        bf_state = bf_nextstate;
        BuildFrame();
        fake_sp = false;
        last_fsp = false;
      } else if(ack_check){
        if(bitstuff_out != 0) error_f = true;
        //Serial.print("ack: ");
        //Serial.print(bitstuff_out);
        //Serial.print(" ");
        ack_check = false;
        ack_sender = false;
        Writing = true;
        bitstuff_in = 1; // ack delimiter
        bf_state = ACK_d;
        bf_nextstate = Eof;
      }
    }
  }
}
