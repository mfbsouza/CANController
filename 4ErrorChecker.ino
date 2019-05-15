/* ERROR CHECKER UNIT */

// inputs: bitstuff_out, fakesp, bf_state
// outputs: sets error_f flag or not

void ErrorChecker(){
  
  switch(bf_state){

    // Format errors
    case CRC_d:
      if(bitstuff_out == 0) error_f = true;
      break;
    case ACK_d:
      if(bitstuff_out == 0) error_f = true;
      break;
    case Eof:
      if(bitstuff_out == 0) error_f = true;
      break;
    case IDE:
      if(bitstuff_out == 1 && tmp_buff.srr == 0) error_f = true;
      break;
  }
}
