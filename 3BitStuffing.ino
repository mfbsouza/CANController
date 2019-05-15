/* bit stuffing memory */

byte last_sb   = 3;
byte last_send = 3;
byte can_tx    = 1;

enum bs_states {
  idle_, loop_, stuff
};


byte bs_state = idle_;
byte counter  = 0;


void BitStuffing(){

  if(!bitstuff_on){
    //can_tx = bitstuff_in;
    //Serial.print(bitstuff_in);
    digitalWrite(CANTX, !bitstuff_in);
    bitstuff_out = sample_bit;
    //Serial.print(bitstuff_out);
    fake_sp = sample_point;
    fake_wp = write_point;
    // reset
    bs_state = idle_;
    last_sb = bitstuff_out;
    last_send = 3;
    counter = 0;
    
  } else if(!Writing) {

    switch(bs_state) {
      
      case idle_:
      
        bitstuff_out = sample_bit;
        //Serial.print(bitstuff_out);
        fake_sp = true;
        if(last_sb == sample_bit) {
          counter = 2;
          bs_state = loop_;
        } else {
          bs_state = idle_;
        }
        last_sb = sample_bit;
        break;

      case loop_:
      
        // //bitstuff_out = sample_bit;
        // //fake_sp = true;
        
        if(last_sb == sample_bit){
          if(counter < 5){
            counter++;
            bitstuff_out = sample_bit;
            //Serial.print(bitstuff_out);
            fake_sp = true;
            bs_state = loop_;
            if(counter == 5){
              bs_state = stuff;
            }
          }
        } else {
          bitstuff_out = sample_bit;
          //Serial.print(bitstuff_out);
          fake_sp = true;
          counter = 0;
          bs_state = idle_;
        }
        last_sb = sample_bit;
        break;

      case stuff:
      
        if(last_sb == sample_bit){
          error_f = true;
          fake_sp = true; // tell build frame about the error
        } else {
          counter = 0;
          bs_state = idle_;
        }
        last_sb = sample_bit;
        break;
    }
  
  } else if(Writing) {

    switch(bs_state){
      case idle_:
        //can_tx = bitstuff_in;
        //Serial.print(bitstuff_in);
        digitalWrite(CANTX, !bitstuff_in);
        fake_wp = true;
        if(last_send == bitstuff_in){
          counter = 2;
          bs_state = loop_;
        } else {
          bs_state = idle_;
        }
        last_send = bitstuff_in;
        break;
        
      case loop_:
        //can_tx = bitstuff_in;
        //fake_wp = true;
        
        if(last_send == bitstuff_in){
          if(counter < 5){
            counter++;
            //can_tx = bitstuff_in;
            //Serial.print(bitstuff_in);
            digitalWrite(CANTX, !bitstuff_in);
            fake_wp = true;
            bs_state = loop_;
            if(counter == 5){
              bs_state = stuff;
            }
          }
        } else {
          //can_tx = bitstuff_in;
          //Serial.print(bitstuff_in);
          digitalWrite(CANTX, !bitstuff_in);
          fake_wp = true;
          counter = 0;
          bs_state = idle_;
        }
        last_send = bitstuff_in;
        break;

      case stuff:
        if(last_send == 1){
          can_tx = 0;
          digitalWrite(CANTX, !LOW);
        } else {
          can_tx = 1;
          digitalWrite(CANTX, !HIGH);
        }
        last_send = can_tx;
        bs_state = idle_;
        break;
    }
  }
  
}
