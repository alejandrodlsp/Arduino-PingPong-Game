#include <FastLED.h>
#include <SoftwareSerial.h> 

// ------------------------------------------------- SOUND DEFINES -------------------------------------------
#define NEXT_SONG 0X01 
#define PREV_SONG 0X02 

#define CMD_PLAY_W_INDEX 0X03 //DATA IS REQUIRED (number of song)

#define VOLUME_UP_ONE 0X04
#define VOLUME_DOWN_ONE 0X05
#define CMD_SET_VOLUME 0X06//DATA IS REQUIRED (number of volume from 0 up to 30(0x1E))
#define SET_DAC 0X17
#define CMD_PLAY_WITHVOLUME 0X22 //data is needed  0x7E 06 22 00 xx yy EF;(xx volume)(yy number of song)

#define CMD_SEL_DEV 0X09 //SELECT STORAGE DEVICE, DATA IS REQUIRED
                #define DEV_TF 0X02 //HELLO,IM THE DATA REQUIRED
                
#define SLEEP_MODE_START 0X0A
#define SLEEP_MODE_WAKEUP 0X0B

#define CMD_RESET 0X0C//CHIP RESET
#define CMD_PLAY 0X0D //RESUME PLAYBACK
#define CMD_PAUSE 0X0E //PLAYBACK IS PAUSED

#define CMD_PLAY_WITHFOLDER 0X0F//DATA IS NEEDED, 0x7E 06 0F 00 01 02 EF;(play the song with the directory \01\002xxxxxx.mp3

#define STOP_PLAY 0X16

#define PLAY_FOLDER 0X17// data is needed 0x7E 06 17 00 01 XX EF;(play the 01 folder)(value xx we dont care)

#define SET_CYCLEPLAY 0X19//data is needed 00 start; 01 close

#define SET_DAC 0X17//data is needed 00 start DAC OUTPUT;01 DAC no output
////////////////////////////////////////////////////////////////////////////////////

// SOUND INDEX
#define WRONG_SOUND_INDEX 0X0F01
#define SCORE_SOUND_INDEX 0X0F02
#define WIN_SOUND_INDEX 0X0F03
#define MENU_MUSIC_INDEX 0X0F04
#define GAME_SOUND_INDEX 0X0F05

// PINS
#define DATA_PIN 10
#define BUTTON1_PIN 9
#define BUTTON2_PIN 3
#define ARDUINO_RX 5//should connect to TX of the Serial MP3 Player module 
#define ARDUINO_TX 6//connect to RX of the module 

// LEDS
#define NUM_LEDS 9
CRGB leds[NUM_LEDS];

// VARS
#define WIN_SCORE 4
#define START_SECONDS 2
#define SCORE_SECONDS 2
#define WRONG_SECONDS 1
#define SPEED_INCREASE 0.03

// GAME VARS
float speed = .12; //Time of change between LED's in seconds
float minSpeed = .03;
byte player1_score = 0; // PLAYER 1 IS PURPLE
byte player2_score = 0; // PLAYER 2 IS GREEN
bool inGame = false;

//SERIAL SOUND
SoftwareSerial mySerial(ARDUINO_RX, ARDUINO_TX);
static int8_t Send_buf[8] = {0} ;//The MP3 player undestands orders in a 8 int string 

void setup() {  // SETUP
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);

  Serial.begin(9600);
  mySerial.begin(9600);
  delay(500);
  sendCommand(CMD_SEL_DEV, DEV_TF);//select the TF card 
  
  delay(2000);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  sendCommand(CMD_PLAY_WITHVOLUME, MENU_MUSIC_INDEX); //PLAY MENU SONG ON INITIALIZE
}

void loop() { // LOOP 

  if(!inGame){ // !INGAME
      //fill_rainbow( leds, NUM_LEDS, 0, 20);
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

void game(){
     leds[0] = CRGB::Green;
     leds[NUM_LEDS -1] = CRGB::Purple;
     for(int _led = 0; _led < NUM_LEDS; _led ++) {
      leds[0] = CRGB::Green;
      leds[NUM_LEDS -1] = CRGB::Purple;
      leds[_led] = CRGB::White;
      FastLED.show();

      for(int _milis = 0; _milis < speed * 100; _milis ++){ // FOWARDS
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

    for(int _led = NUM_LEDS - 2; _led >= 0; _led --) { // BACKWARDS
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

void buttonPressed(bool _player, int _pos){
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

void showScore(){
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

void win(bool _player2){
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
  inGame = false;
  delay(1000);
  
  sendCommand(CMD_PLAY_WITHVOLUME, MENU_MUSIC_INDEX); //PLAY MENU SONG ON INITIALIZE
}
void wrongClick(){
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
  for(int _x = 0; _x < NUM_LEDS; _x ++) {  // SETS ALL LED BLACK
      leds[_x] = CRGB::Black;
  }
  FastLED.show();
}
void confetti(){   // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( random8(64), 200, 255);
}

void sendCommand(int8_t command, int16_t dat)
{
 delay(20);
 Send_buf[0] = 0x7e; //starting byte
 Send_buf[1] = 0xff; //version
 Send_buf[2] = 0x06; //the number of bytes of the command without starting byte and ending byte
 Send_buf[3] = command; //
 Send_buf[4] = 0x00;//0x00 = no feedback, 0x01 = feedback
 Send_buf[5] = (int8_t)(dat >> 8);//datah
 Send_buf[6] = (int8_t)(dat); //datal
 Send_buf[7] = 0xef; //ending byte
 for(uint8_t i=0; i<8; i++)//
 {
   mySerial.write(Send_buf[i]) ;//send bit to serial mp3
   Serial.print(Send_buf[i],HEX);//send bit to serial monitor in pc
 }
 Serial.println();
}



