//Flappy Birdie
//Version 1.6

//Version 1.6 update notes:
//Added a hard mode!
//In hard mode, the distance between pipes is 2x smaller

//Version 1.5 update notes:
//Added an option to return to the main menu after Game Over!
//Fixed a glitch where after pausing on a frame-perfect location infinitely increases the score!

//Version 1.4.1 update notes:
//Added a license (wow such update)

//Version 1.4 update notes:
//Added a line of code that unmutes the arduboy or turns the sound on.
//The reason is because when using the reset button the arduboy is set to mute.

//Version 1.3 update notes:
//Fixed a warning on pipe collision (thanks to Pharap: https://community.arduboy.com/u/Pharap)
//Changed soundPlay from 8 to 7 when passing through a regular pipe.

//Version 1.2 update notes:
//Fixed the glitch where "New Hi-Score" isn't shown when getting a new Hi-Score
//Added an integer to track if a 'score sound' is playing so the jump sound doesn't overwrite it
//Changed a note on sound when passing score 50

//Version 1.1 update notes:
//Changed the way to control RBG LEDs (thanks to MLXXXp: https://community.arduboy.com/u/MLXXXp)

//License:
// This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
// To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/ or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.


#include <ArduboyTones.h>
#include <ArduboyTonesPitches.h>

#include <Arduboy2.h>
Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);

#include "Images.h"
#include "EEPROMUtils.h"
#include "Sounds.h"

struct Bird {
  int Y;
  int FlyIndex;
  const byte *image;
};
Bird bird = {16,0,bird_0};
int Flying[] = {0,-2,-2,-2,-1,-1,-1,-1,0,0,1,1,1,1,2,2,2,4};
int BackgroundCamera;
int Camera;
bool Playing;
bool FirstTime = true;
int MenuState;
bool ReadyToPlay;
int Score;
int HI;
int DeleteTimer;
int ShowDelTxt;
bool Pause;
bool FailSound;
int soundPlay;
bool placeHi;
bool newHi;
bool arrowY;
bool menuOpen;
bool modeHard;
int He;
int Hh;

struct Pipe {
  bool exist;
  int x;
  int y;
  int level;
  const byte *image;
  const byte *mask;
};
Pipe pipe[6];

void setup() {
  initEEPROM();
  arduboy.begin();
  arduboy.clear();
  arduboy.setFrameRate(30);
  EEPROM.get(EEPROM_SCORE, He);
  EEPROM.get(EEPROM_SCORE + 1, Hh);
  EEPROM.get(EEPROM_SCORE + 2, modeHard);
  HI = modeHard ? Hh : He;
  arduboy.audio.on();
}

bool collision() {
  for(int i; i < 6; i++){
    if(pipe[i].exist == true){
      Rect birdRect = Rect{11,bird.Y+1,12,8};
      Rect pipe1Rect = Rect{pipe[i].x,pipe[i].y,16,32 + pipe[i].level * 2};
      Rect pipe2Rect = Rect{pipe[i].x,pipe[i].y + 68 - pipe[i].level * 2,16,static_cast<uint8_t>(32 + pipe[i].level * 2)};
      if(arduboy.collide(birdRect, pipe1Rect) or arduboy.collide(birdRect, pipe2Rect) or bird.Y > 46) return true;
    }
  }
  return false;
}

void loop() {
  if(!(arduboy.nextFrame())) return;
  if((Playing or FirstTime) and !(Pause)){
    if(arduboy.everyXFrames(2)) BackgroundCamera++;
    Camera++;
  }
  if(soundPlay > 0) soundPlay--;
  if(collision()) Playing = false;
  arduboy.pollButtons();
  arduboy.clear();
  //Draws the background. Place at beggining of code.
  for(int y; y < 3; y++){
    arduboy.drawBitmap(y*64-BackgroundCamera%64,31,background,64,32,WHITE);
  }
  if(FirstTime){
    arduboy.drawBitmap(23,1,logo,59,27,WHITE);
    if(MenuState < 12) MenuState++;
    else MenuState = 0;
    if(MenuState == 0) bird.image = bird_1;
    if(MenuState == 4) bird.image = bird_2;
    if(MenuState == 8) bird.image = bird_0;
    arduboy.setCursor(1,30);
    arduboy.print("Press A to play!\nHi-Score: ");
    arduboy.print(HI);
    arduboy.print("\nHold B to delete data");
    if(arduboy.pressed(B_BUTTON) and HI > 0) DeleteTimer++;
    if(arduboy.notPressed(B_BUTTON)) {DeleteTimer = 0; arduboy.digitalWriteRGB(RED_LED, RGB_OFF);}
    if(arduboy.justPressed(A_BUTTON)) {
      FirstTime = false;
      MenuState = 0;
      Playing = true;
      menuOpen = false;
    }if(arduboy.everyXFrames(15)) arrowY = !(arrowY);
    arduboy.drawBitmap(110,arrowY,arrow,16,16,WHITE);
    if(arduboy.justPressed(UP_BUTTON)) menuOpen = !(menuOpen);
  }
  if(arduboy.justPressed(B_BUTTON) and Playing) Pause = !(Pause);
  if(Playing and !(Pause)){
    if(arduboy.justPressed(A_BUTTON)) {
      bird.FlyIndex = 0;
      bird.image = bird_1;
      if(soundPlay == 0) sound.tones(soundFlying);
    }
    if(bird.FlyIndex < 17) bird.FlyIndex++;
    if(bird.FlyIndex == 8) bird.image = bird_2;
    if(bird.FlyIndex == 12) bird.image = bird_0;
    if(bird.Y + Flying[bird.FlyIndex] < 0)  bird.Y = 0;
    else bird.Y = bird.Y + Flying[bird.FlyIndex];
    if(arduboy.everyXFrames(60 / (modeHard + 1))){
      for(int z; z < 6; z++){
        if(!(pipe[z].exist)){
          pipe[z].exist = true;
          pipe[z].x = 126;
          pipe[z].y = random(0,32) - 32;
          if(Score < 11){
            pipe[z].level = 0;
            pipe[z].image = pipes_0;
            pipe[z].mask = pipes_mask_0;
          }
          else if(Score < 21){
            pipe[z].level = 1;
            pipe[z].image = pipes_1;
            pipe[z].mask = pipes_mask_1;
          }
          else if(Score < 51){
            pipe[z].level = 2;
            pipe[z].image = pipes_2;
            pipe[z].mask = pipes_mask_2;
          }else{
            pipe[z].level = 3;
            pipe[z].image = pipes_3;
            pipe[z].mask = pipes_mask_3;
          }
          break;
        }
      }
    }
  }
  for(int a; a < 6; a++){
    if(pipe[a].exist){
      if(Playing and !(Pause)) pipe[a].x--;
      if(pipe[a].x < -15) pipe[a].exist = false;
      Sprites::drawExternalMask(pipe[a].x,pipe[a].y,pipe[a].image,pipe[a].mask,0,0);
      if(pipe[a].x == 9 and Playing and !(Pause)){
        Score++;
        if(Score == 10) {sound.tones(soundSuper1); soundPlay = 14;}
        else if(Score == 20) {sound.tones(soundSuper2); soundPlay = 17;}
        else if(Score == 50) {sound.tones(soundSuper3); soundPlay = 24;}
        else {sound.tones(soundScore); soundPlay = 7;}
      }
    }
  }
  arduboy.setCursor(24,4);
  if(Playing) {arduboy.print("Score: ");arduboy.print(Score);}
  //Draws the ground. Place at end of code.
  for(int x; x < 9; x++){
    Sprites::drawOverwrite(x*16-Camera%16,56,ground,0);
  }
  if(DeleteTimer > 0 and FirstTime){
    arduboy.digitalWriteRGB(RED_LED, RGB_ON);
    arduboy.drawRect(0,56,104,7,BLACK);
    arduboy.drawRect(1,57,102,5,WHITE);
    arduboy.fillRect(2,58,DeleteTimer*2,3,WHITE);
    if(DeleteTimer == 50){
      DeleteTimer = 0;
      HI = 0;
      He = 0;
      Hh = 0;
      EEPROM.put(EEPROM_SCORE, (unsigned int)0);
      EEPROM.put(EEPROM_SCORE + 1, (unsigned int)0);
      EEPROM.put(EEPROM_SCORE + 2, (unsigned int)0);
      ShowDelTxt = 90;
    }
  }
  if(ShowDelTxt > 0 and FirstTime){
    arduboy.setCursor(1,55);
    arduboy.print("\nHi-Score deleted!");
    ShowDelTxt--;
  }
  //Draws the bird. Place after the end of code ;)
  Sprites::drawExternalMask(10,bird.Y,bird.image,bird_mask,0,0);
  if(!(Playing) and !(FirstTime)){
    if(!(ReadyToPlay) and arduboy.everyXFrames(30)) ReadyToPlay = true;
    if(!(FailSound)) {
      if(Score > HI) sound.tones(soundNewHI);
      else sound.tones(soundFall);
      FailSound = true;
    }
    arduboy.drawRect(0,15,128,47,WHITE);
    arduboy.fillRect(1,16,126,45,BLACK);
    arduboy.setCursor(10,17);
    arduboy.print("Game Over");
    arduboy.setCursor(2,25);
    if(Score > HI and !(placeHi)) {
      if(modeHard) {
        EEPROM.put(EEPROM_SCORE + 1, Score);
        Hh = Score;
        HI = Hh;
      } else {
        EEPROM.put(EEPROM_SCORE, Score);
        He = Score;
        HI = He;
      }
      placeHi = true;
      newHi = true;
    }
    if(newHi) arduboy.print("New Hi-Score!");
    else {arduboy.print("Hi-Score: ");arduboy.print(HI);}
    arduboy.setCursor(2,34);
    arduboy.print("Score: ");
    arduboy.print(Score);
    arduboy.setCursor(2,43);
    if(ReadyToPlay){
      arduboy.print("Press A to try again");
      arduboy.setCursor(2,52);
      arduboy.print("Press B to return");
      if(arduboy.justPressed(A_BUTTON)){
        Playing = true;
        for(int p; p < 6; p++){
          pipe[p].exist = false;
        }
        Camera = 0;
        bird.Y = 16;
        bird.FlyIndex = 0;
        bird.image = bird_0;
        Score = 0;
        ReadyToPlay = false;
        FailSound = false;
        placeHi = false;
        newHi = false;
      }if(arduboy.justPressed(B_BUTTON)) {
        for(int p; p < 6; p++) {
          pipe[p].exist = false;
        }Camera = 0;
        bird.Y = 16;
        FirstTime = true;
        bird.FlyIndex = 0;
        bird.image = bird_0;
        Score = 0;
        ReadyToPlay = false;
        FailSound = false;
        placeHi = false;
        newHi = false;
      }
    }
  }
  if(Pause){
    arduboy.setCursor(1,1);
    arduboy.print("Paused");
  }if(menuOpen) {
    if(arduboy.justPressed(RIGHT_BUTTON)) {modeHard = !(modeHard);EEPROM.put(EEPROM_SCORE + 2, modeHard);HI = modeHard ? Hh : He;}
    arduboy.fillRect(0,16,128,36,BLACK);
    arduboy.setCursor(0,16);
    arduboy.print(F("Press RIGHT for mode\n"));
    arduboy.print(modeHard ? "Hard" : "Easy");
    arduboy.print(F(" Mode\nMore features\ncoming soon!"));
  }
  arduboy.display();
}
