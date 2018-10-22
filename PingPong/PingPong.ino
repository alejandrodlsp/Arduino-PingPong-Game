//------------------------------------
//  ALEJANDRO DE LOS SANTOS PUERTO
//  ALEJANDRODLSP@HOTMAIL.ES
//  @ALEJANDRODLSP
//  10/9/2018
//------------------------------------
// AN ARDUINO REFLEX MINIGAME USING A SERIAL MP3 PLAYER MODULE AND ws2801b LED's
//

#include <FastLED.h>
#include <SoftwareSerial.h> 

// ------------------------------------------------- SOUND DEFINES -------------------------------------------
#define CMD_PLAY_W_INDEX 0X03 // INDEX OF SONG REQUIRED

#define CMD_SET_VOLUME 0X06 //DATA REQUIRED
#define CMD_PLAY_WITHVOLUME 0X22 //INDEX OF SONG REQUIRED, VOLUME REQUIRED

#define CMD_SEL_DEV 0X09 //SELECT STORAGE DEVICE, DATA REQUIRED
#define DEV_TF 0X02 // DATA REQUIRED
////////////////////////////////////////////////////////////////////////////////////

// SOUND INDEX IN MP3 PLAYER SD CARD
#define WRONG_SOUND_INDEX 0X0F01
#define SCORE_SOUND_INDEX 0X0F02
#define WIN_SOUND_INDEX 0X0F03
#define MENU_MUSIC_INDEX 0X0F04
#define GAME_SOUND_INDEX 0X0F05

// PINS
#define DATA_PIN 10
#define BUTTON1_PIN 9 // PIN CONNECTED TO PLAYER 1 BUTTON
#define BUTTON2_PIN 3 // PIN CONNECTED TO PLAYER 2 BUTTON
#define ARDUINO_RX 5  // TX OF SERIAL MP3 PLAYER
#define ARDUINO_TX 6  // RX OF SERIAL MP3 PLAYER

// LEDS
#define NUM_LEDS 9  // LED NUMBER
CRGB leds[NUM_LEDS]; 

// VARS
#define WIN_SCORE 4 // SCORE NEEDED TO WIN
#define START_SECONDS 2 // SECONDS 
#define SCORE_SECONDS 2 // SECONDS SHOWING SCORE
#define WRONG_SECONDS 1 // SECONDS IN WRONG CLICK SCREEN
#define SPEED_INCREASE 0.033  // SPEED INCREASE PER SCORE
#define minSpeed  0.022  // MINIMUM GAME SPEED

// GAME VARS
float speed = .085; //Time of change between LED's in seconds
byte player1_score = 0; // PLAYER 1 IS PURPLE
byte player2_score = 0; // PLAYER 2 IS GREEN
bool inGame = false;

//SERIAL SOUND
SoftwareSerial mySerial(ARDUINO_RX, ARDUINO_TX);  // SOFTWARE SERIAL PROTOCOL
static int8_t Send_buf[8] = {0} ;

void setup() {  // SETUP
  pinMode(BUTTON1_PIN, INPUT);  // SET BUTTONS TO INPUTS
  pinMode(BUTTON2_PIN, INPUT);

  Serial.begin(9600); // START SERIAL COMMUNICATIONS
  mySerial.begin(9600);
  delay(500);
  
  sendCommand(CMD_SEL_DEV, DEV_TF); //select the TF card 
  
  delay(2000);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  sendCommand(CMD_PLAY_WITHVOLUME, MENU_MUSIC_INDEX); //PLAY MENU SONG ON INITIALIZE
}

void loop() { // LOOP 

  if(!inGame){ // !INGAME, DEAL WITH MAIN MANU LOGIC
      confetti();
      FastLED.show();
      delay(30);
      if(digitalRead(BUTTON1_PIN) == HIGH && digitalRead(BUTTON2_PIN) == HIGH){ // IF BOTH BUTTONS PRESSED
          delay(START_SECONDS * 1000); // DELAY FOR X SECONDS
          if(digitalRead(BUTTON1_PIN) == HIGH && digitalRead(BUTTON2_PIN) == HIGH){   // THEN CHECK IF THEY"RE STILL PRESSED
             inGame = true;                                                         // START GAME
             allBlack();
             wrongClick();
             return;
           }
       }
   }  // !INGAME
   
   if(inGame){// INGAME
      game();
   }// INGAME
    
} // LOOP

void game(){  // DEALS WITH GAME LOGIC
     leds[0] = CRGB::Green; // SET PLAYER LEDS COLOR
     leds[NUM_LEDS -1] = CRGB::Purple;
     
     for(int _led = 0; _led < NUM_LEDS; _led ++) {  // FORWARDS LOOP
      leds[0] = CRGB::Green;
      leds[NUM_LEDS -1] = CRGB::Purple;
      leds[_led] = CRGB::White;
      FastLED.show();

      for(int _milis = 0; _milis < speed * 100; _milis ++){
        delay(10);
        if(digitalRead(BUTTON1_PIN) == HIGH){
          buttonPressed(false, _led);
          _led = random(NUM_LEDS);
        }
        if(digitalRead(BUTTON2_PIN) == HIGH){
          buttonPressed(true, _led);
          _led = random(NUM_LEDS);
        }
        
      }
      // Turn our current led back to black for the next loop around
      leds[_led] = CRGB::Black;
    }  // FOWARDS LOOP

    for(int _led = NUM_LEDS - 2; _led >= 0; _led --) { // BACKWARDS LOOP
      leds[0] = CRGB::Green;
      leds[NUM_LEDS -1] = CRGB::Purple;
      leds[_led] = CRGB::White;
      FastLED.show();

      for(int _milis = 0; _milis < speed * 100; _milis ++){
        delay(10);
        if(digitalRead(BUTTON1_PIN) == HIGH){
          buttonPressed(false, _led);
          _led = random(NUM_LEDS);
        }
        if(digitalRead(BUTTON2_PIN) == HIGH){
          buttonPressed(true,_led);
          _led = random(NUM_LEDS);
        }
        
      }
      // Turn our current led back to black for the next loop around
      leds[_led] = CRGB::Black;
      
   }  // BACKWARDS LOOP
} // GAME

void buttonPressed(bool _player, int _pos){ // WHEN A BUTTON IS PRESSED
    if(!_player){ // IF PLAYER1
      if(_pos == NUM_LEDS -1){  // IF IN PLAYER's WIN POS
          player1_score ++;
          showScore();
          return;
       }else{
        wrongClick();
        }
    }

    if(_player){  // IF PLAYER 2
      if(_pos == 0){    // IF IN PLAYER's WIN POS
        player2_score ++; 
        showScore();
        return;
      }else{
        wrongClick();
        }
    }  
} // BUTTON PRESSED

void showScore(){ // SHOWS SCORE AND DEALS WITH SCORE MANAGMENT
    if(player2_score == WIN_SCORE){
      win(true);
      return;  
    }
    if(player1_score == WIN_SCORE){
      win(false);
      return;  
    }
    
     sendCommand(CMD_PLAY_WITHVOLUME, SCORE_SOUND_INDEX); //PLAY MENU SONG ON INITIALIZE
     
     if(speed - SPEED_INCREASE >= minSpeed){
         speed -= SPEED_INCREASE;
     }else{
        speed = minSpeed;
     }
     
     allBlack();
     leds[NUM_LEDS - 1] = CRGB::Purple;
     leds[0] = CRGB::Green;
     leds[4] = CRGB::Red;
     for(int whiteLed = 1; whiteLed <= player2_score; whiteLed ++) {
      leds[whiteLed] = CRGB::Yellow;
     }
     for(int whiteLed = NUM_LEDS - 2; whiteLed >= NUM_LEDS - 1 - player1_score; whiteLed --) {
      leds[whiteLed] = CRGB::Blue;
     }
     FastLED.show();

  // Turn our current led back to black for the next loop around
  delay(SCORE_SECONDS * 1000);
  allBlack();
  
} // SHOW SCORE

void win(bool _player2){  // WIN SEQUENCE
  sendCommand(CMD_PLAY_WITHVOLUME, WIN_SOUND_INDEX); //PLAY WRONG SOUND ON INITIALIZE
  if(_player2){
    for(int r=0;r<=6;r++){  // REPEAT WIN SEQUENCE X TIMES
      for(int _x = 0; _x < NUM_LEDS; _x ++) {  // SETS ALL LED RED
       leds[_x] = CRGB::Green;
      }
      FastLED.show();
      delay(1000); 
      allBlack();
      delay(500);
    }
  }
  if(!_player2){
    for(int r=0;r<=6;r++){  // REPEAT WIN SEQUENCE X TIMES
      for(int _x = 0; _x < NUM_LEDS; _x ++) {  // SETS ALL LED RED
       leds[_x] = CRGB::Purple;
      }
      FastLED.show();
      delay(1000); 
      allBlack();
      delay(500);
    }
  }
  inGame = false; // RESET GAME VALUeS
  player1_score = 0; 
  player2_score = 0;
  speed = .085;
  delay(1000);
  
  sendCommand(CMD_PLAY_WITHVOLUME, MENU_MUSIC_INDEX); //PLAY MENU SONG ON INITIALIZE
}

void wrongClick(){  // WRONG CLICK SEQUENCE
  sendCommand(CMD_PLAY_WITHVOLUME, WRONG_SOUND_INDEX); //PLAY WRONG SOUND ON INITIALIZE
  for(int _x = 0; _x < NUM_LEDS; _x ++) {  // SETS ALL LED RED
      leds[_x] = CRGB::Red;
  }
  FastLED.show();
  
  delay(WRONG_SECONDS * 1000/2);
  allBlack();
  delay(300);
  
  for(int _x = 0; _x < NUM_LEDS; _x ++) {  // SETS ALL LED BLACK
      leds[_x] = CRGB::Red;
  }
  FastLED.show();
  
  delay(WRONG_SECONDS * 1000/2);
  allBlack();
}

void allBlack(){
  for(int _x = 0; _x < NUM_LEDS; _x ++) {  // SETS ALL LEDs BLACK
      leds[_x] = CRGB::Black;
  }
  FastLED.show();
}
void confetti(){   // Random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( random8(64), 200, 255);
}

void sendCommand(int8_t _command, int16_t _data)
{
 delay(20);
 Send_buf[0] = 0x7e; // START BYTE
 Send_buf[1] = 0xff;
 Send_buf[2] = 0x06; // NUMBER OF BYTES IN COMMAND
 Send_buf[3] = _command; 
 Send_buf[4] = 0x00; // NO FEEDBACK REQUIRED
 Send_buf[5] = (int8_t)(_data >> 8);// DATAH
 Send_buf[6] = (int8_t)(_data); // DATAL
 Send_buf[7] = 0xef; //END BYTE
 
 for(uint8_t z=0; z<8; z++)//
 {
   mySerial.write(Send_buf[z]) ; // SEND BIT
 }
}



