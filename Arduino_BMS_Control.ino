#include "HID-Project.h"

//common
char keys[11] = {'i', 'e', 'z', 'x', 'c', 'f', 'g', 'h', 'j', 'q', 'w'};
unsigned long elapsed = 0;

//scratch
const unsigned long ACTIVE_TIME = 150000; //us
const int THRESHOLD = 750;
const int THRES_LOW = 850;
const int THRES_HIGH = 650;
long scrTimer = 0;
bool toRight;
int d1f, d2f;
int list[8][4] = {
  {1, 1, 0, 1},
  {1, 0, 1, 1},
  {0, 0, 1, 0},
  {0, 1, 0, 0},
  {1, 1, 1, 0},
  {1, 0, 0, 0},
  {0, 1, 1, 1},
  {0, 0, 0, 1}
};

//keys
const unsigned long INTERVAL = 10000;
unsigned long ptime[9];

void setup() {
  //common
  NKROKeyboard.begin();
  for (int i = 2; i < 11; i++){ //set pinmode
    pinMode(i, INPUT_PULLUP);
  }
  elapsed = micros();

  //scratch
  toRight = false;
  d1f = analogRead(A1) < THRESHOLD ? 0 : 1;
  d2f = analogRead(A2) < THRESHOLD ? 0 : 1;

  //keys
  for (int i = 0; i < 9; i++){
    ptime[i] = 0;
  }

}

void loop() {
  //common
  unsigned long now = micros();
  if (now < elapsed) { //overflow!
    elapsed = now;
    return;
  }
  long tdif = now - elapsed;
  elapsed = now;

  //scratch
  int d1l, d2l;
  int thres1 = d1f == 0 ? THRES_LOW : THRES_HIGH;
  int thres2 = d2f == 0 ? THRES_LOW : THRES_HIGH;
  d1l = analogRead(A1) < thres1 ? 0 : 1;
  d2l = analogRead(A2) < thres2 ? 0 : 1;

  bool triggered = false;
  int state[4] = {d1f, d1l, d2f, d2l};
  for (int i = 0; i < 8; i++) {
    if (memcmp(list[i], state, sizeof(int) * 4) == 0) { //compare
      scrTimer = ACTIVE_TIME;
      triggered = true;
      toRight = i < 4;
      break;
    }
  }
  if (!triggered) {
    scrTimer -= tdif;
  }
  if (scrTimer > 0) {
    if (toRight) {
      //Right Turn
      NKROKeyboard.add(keys[0]);
      NKROKeyboard.remove(keys[1]);
    } else {
      //Left Turn
      NKROKeyboard.add(keys[1]);
      NKROKeyboard.remove(keys[0]);
    }
  } else {
    //No Input
    scrTimer = 0;
    NKROKeyboard.remove(keys[0]);
    NKROKeyboard.remove(keys[1]);
  }
  d1f = d1l;
  d2f = d2l;

  //keys
  for (int i = 0; i < 9; i++) {
    ptime[i] = min(ptime[i] + tdif, INTERVAL);
    if (!digitalRead(i + 2)) {
      NKROKeyboard.add(keys[i + 2]);
      ptime[i] = 0;
    } else {
      if (ptime[i] < INTERVAL){ //burst!
        continue;
      }
      NKROKeyboard.remove(keys[i + 2]);
    }
  }

  NKROKeyboard.send();
}
