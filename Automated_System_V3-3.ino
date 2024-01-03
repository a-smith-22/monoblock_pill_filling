/*
TITLE: Automated Monoblock Pill Filling System Control
AUTHORS: Andrew Smith, Sibyl Roosen
DATE: May 11, 2023 (last updated)
NOTES: https://docs.arduino.cc/learn/electronics/stepper-motors
*/

// Rotary Table
const int DIRpinROT = 3;   // define direction (DIR+) pin
const int PULpinROT = 4;   // define pulse (PUL+) pin 
const int spd_ROT = 2500; // set rotary table speed (delay between pulses)
const int spd_ROT_ADJ = spd_ROT; // set rotary table adjustment speed (for pill filling), use same as normal speed
int adj_bottle = 1; // boolean statement to adjust bottle once in pill filling
const int adj_bot_dst = 140; // number of steps to adjust bottle


// Pill Filling
const int DIRpinPF = 11;   // define direction (DIR+) pin
const int PULpinPF = 2;   // define pulse (PUL+) pin
const int spd_PF = 1500; // set pill filling speed
const int tube_step = 540; // number of steps to count to rotate between pill tubes
int pos = 0; // current steps of pill filling system 
int fill_bottle = 1;

// Pill Shaking
float pos_hat = 0; // TEMPORARY VARIABLE: adjusted position -> 0 for center, 1 for next pill tube over
int pill_dir = 1; // pill filling system going forward or backward

// Pill Counting
int wait_time = 0; //waiting variable to fill pills
int total_wait = 600; // number of frames to wait for pill filling

// Button Control
const int ON_PIN = 6; // "on" button, left (green)
const int OFF_PIN = 7; // emergency "off" Button (yellow)
const int BOT_LT_PIN = 5; // pin for the bottle sensor light (green)
const int ON_LT_PIN = 10; // pin for the "on" indicator light (red)
int system_on = 0; // boolean to run system or not (1 = on, 0 = off)

// Sensors
const int bot_sns_pin = 8; // pin for bottle sensor

void setup() {
  // Rotary Table
  pinMode(DIRpinROT,OUTPUT); // set DIRpin as output
  pinMode(PULpinROT,OUTPUT); // set PULpin as output
  digitalWrite(DIRpinROT,LOW);    // set direction forward, counter-clockwise 
  digitalWrite(PULpinROT,LOW); // start with no power

  // Pill Filling
  pinMode(DIRpinPF,OUTPUT); // set DIRpinPF as output
  pinMode(PULpinPF,OUTPUT); // set PULpinPF as output
  digitalWrite(DIRpinPF,LOW);    // set direction forward, counter-clockwise 
  digitalWrite(PULpinPF,LOW); // start with no power

  // Sensors
  pinMode(bot_sns_pin, INPUT_PULLUP);    //sensor bottle

  // Button Control
  pinMode(ON_PIN, INPUT_PULLUP); // initialize on button
  pinMode(OFF_PIN, INPUT_PULLUP); // initiailize off button
  pinMode(BOT_LT_PIN,OUTPUT); // set pill sensor pin as output
  pinMode(ON_LT_PIN,OUTPUT); // set pill sensor pin as output

  // Misc.
  Serial.begin(9600); // initiate serial monitor -> set processing rate to 9600
  //display with CTRL+SHIFT+M
}

void loop() {
  // DISPLAY PROCESS
  /*
  pos_hat = float(pos) / tube_step; // TEMPORARY: define adjusted pill filling position
  Serial.println(pos_hat);
  */

  //==================================================================================
  
  // BUTTON CONTROL
  if(digitalRead(ON_PIN) == HIGH) { // "on" button pressed
    system_on = 1; // turn the system on
  }
  if(digitalRead(OFF_PIN) == HIGH) { // "off" button pressed
    system_on = 0; // turn the system off
  }

  // SENSOR LIGHT
  if(digitalRead(bot_sns_pin) == LOW) { // BOTTLE SENSOR is blocked
    //digitalWrite(BOT_LT_PIN, HIGH); // turn on light
  } else {
    digitalWrite(BOT_LT_PIN, LOW); // turn light off elsewise
  }

  //==================================================================================

  // SYSTEM CONTROL -> RUN EVERYTHING
  if(system_on == 1) {
    digitalWrite(ON_LT_PIN, HIGH); // turn indicator light ON

    if (digitalRead(bot_sns_pin) == HIGH) {  // BOTTLE IS NOT IN SENSOR
      // Reset pill filling variables
      adj_bottle = 1;  // set true to adjust next bottle
      wait_time = 0; //reset pill filling wait
      fill_bottle = 1; // set true to fill next bottle

            
      // MOVE ROTARY TABLE & PILL FILLING (PART 1)
      digitalWrite(PULpinROT, HIGH);  // rotary table HIGH      
      digitalWrite(PULpinPF, HIGH);  // pill filling HIGH
      delayMicroseconds(spd_ROT/2); // set speed     
      digitalWrite(PULpinPF, LOW);  // pill filling HIGH
      delayMicroseconds(spd_ROT/2); // set speed     

      // SHAKE PILL FILLING
      // Determine direction of pill filling
      if( abs(pos) > tube_step*0.1 ) {
        pill_dir *= -1; // change direction
      }
      // Set directions
      if(pill_dir > 0) { digitalWrite(DIRpinPF, LOW); } // forward (CW) direction
      if(pill_dir < 0) { digitalWrite(DIRpinPF, HIGH); } // backwards (CCW) direction
      pos += pill_dir; //track pill filling position

      // MOVE ROTARY TABLE & PILL FILLING (PART 2)
      digitalWrite(PULpinROT, LOW);  // rotary table HIGH      
      digitalWrite(PULpinPF, HIGH);  // pill filling HIGH
      delayMicroseconds(spd_ROT/2); // set speed     
      digitalWrite(PULpinPF, LOW);  // pill filling HIGH
      delayMicroseconds(spd_ROT/2); // set speed   
      
      //==================================================================================


    } 
    if( digitalRead(bot_sns_pin) == LOW) {  // BOTTLE IS IN SENSOR
      digitalWrite(BOT_LT_PIN, HIGH); // turn on light
      // Adjust bottle to be in middle of pill filling station
      if (adj_bottle == 1) {  // Move system a little bit
        for (int i = 0; i < adj_bot_dst; i++) { // rotate table a few steps
          digitalWrite(PULpinROT, HIGH);  // output high
          delayMicroseconds(spd_ROT);     // set rotate speed
          digitalWrite(PULpinROT, LOW);   // output low
          delayMicroseconds(spd_ROT);     // set rotate speed
        }
        adj_bottle = 0;  // turn off, don't keep moving bottle. this doesnt change until bottle is out of system
      }

      /*
      // Adjust pill filling to be back to middle
      if ( adj_pill == 1 ) { // ADJUST PILL FILLING POSITION
        if(pos < 1) {  // position is left of desired
          digitalWrite(DIRpinPF, LOW); // move forwards (CCW)
          pill_dir = 1;
        } 
        if(pos > -1) {  // position is right of desired
          digitalWrite(DIRpinPF, HIGH); // move backwards (CW)
          pill_dir = -1;
        }
        if(pos > -1 && pos < 1) { // add tolerance for when pill filling is "zeroed" out
          adj_pill = 0; // stop adjusting once zeroed out
        }
        // Move pill filling
        digitalWrite(PULpinPF, HIGH);  // output high
        delayMicroseconds(spd_PF*3);     // set rotate speed
        digitalWrite(PULpinPF, LOW);   // output low
        delayMicroseconds(spd_PF*3);     // set rotate speed
        pos += pill_dir; // update position

      } 
      */
      /*
      if( adj_pill == 0 ) { // IF PILL SYSTEM IS ADJUSTED, FILL BOTTLE
        // Move pill filling exactly 1 tube over
        digitalWrite(DIRpinPF,LOW); // only move in forward (CCW) direction
        if (pos < tube_step && fill_bottle == 1) {
          digitalWrite(PULpinPF, HIGH);  // output high
          delayMicroseconds(spd_PF*3);     // set rotate speed
          digitalWrite(PULpinPF, LOW);   // output low
          delayMicroseconds(spd_PF*3);     // set rotate speed
          pos += 1;
        }
      } 
      */

      // FILL BOTTLE WITH PILLS
      if( fill_bottle == 1 ) { // Only fill bottle once
        // Set direction to move to next pill tube, only if not at correct position
        if (pos < tube_step) {
          digitalWrite(DIRpinPF,LOW); // move in forward (CW) direction
          pill_dir = 1;
        } 
        if (pos > tube_step) {
          digitalWrite(DIRpinPF,HIGH); // move in backward (CCW) direction
          pill_dir = -1;
        }
        if (pos == tube_step) {
          fill_bottle = 0;
        }
        // move pill filling system 
        digitalWrite(PULpinPF, HIGH);  // output high
        delayMicroseconds(spd_PF);     // set rotate speed
        digitalWrite(PULpinPF, LOW);   // output low
        delayMicroseconds(spd_PF);     // set rotate speed
        // track position
        pos += pill_dir;
      } 

      wait_time += 1; //increase wait_time
      if (wait_time > total_wait) {  // WAIT TIME IS EXCEEDED, MOVE SYSTEM
        // Move bottle out of system, unblock sensor to restart process
        digitalWrite(PULpinROT, HIGH);  // output high
        delayMicroseconds(spd_ROT);     // set rotate speed
        digitalWrite(PULpinROT, LOW);   // output low
        delayMicroseconds(spd_ROT);     // set rotate speed
        fill_bottle = 0; // prevents movement after bottle is filled but still in sensor
        pos = 0; // reset position
        adj_bottle = 0; // reset bottle adjust
      } 
    } 
      
  //==================================================================================


  } 
  if (system_on == 0) { // system is OFF
    digitalWrite(ON_LT_PIN, LOW); // turn indicator light OFF
    // REsSET VARIABLES
    pos = 0; // reset pill filling position
    pill_dir = 1; // reset pill filling direction
    adj_bottle = 1;  // set true to adjust next bottle
    wait_time = 0; //reset pill filling wait
  }

  
} // END OF MAIN LOOP
