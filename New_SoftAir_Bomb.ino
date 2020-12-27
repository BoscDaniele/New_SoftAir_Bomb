#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'a'}, {'4', '5', '6', 'b'}, {'7', '8', '9', 'c'}, {'*', '0', '#', 'd'}
};

byte rowPins[ROWS] = {
  A0, A1, A2, A3
};

byte colPins[COLS] = {
  A4, A5, 13, 12
};

Keypad pad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

byte up[8] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B11111,
  B00000,
  B00000,
};

byte down[8] = {
  B00000,
  B00000,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
};

unsigned long adesso;
unsigned long max_time = 10000;
unsigned long last_time_press;

int state;

bool initial_psw;
bool final_psw;

static char psw[5] = "1234";
char usr_psw[5];

static char super_user_psw[5] = "1234";

int index;

int menu;

void setup() {
  EEPROM.get(0, initial_psw);
  EEPROM.get(1, final_psw);

  pad.setHoldTime(500);
  pad.setDebounceTime(100);
  pad.addEventListener(keypadEvent);

  last_time_press = max_time;
  menu = 0;
  res();
  lcd.begin(16, 2);

  lcd.createChar(0, up);
  lcd.createChar(1, down);
}

void loop() {
  lcd.clear();

  switch (state) {
    case 0:
      if (initial_psw) {
        stamp("Inserisci  Psw", 0);
        stamp("per Armare", 1);

        if (pad.getKey() == 'a')
          state = 1;

      } else {
        stamp("Premi  'A'", 0);
        stamp("per Armare", 1);

        if (pad.getKey() == 'a') {
          adesso = millis();
          state = 2;
        }
      }
      break;

    case 1: {
        stamp("Inserisci  Psw", 0);

        char key = pad.getKey();

        if (key != NO_KEY) {
          if (key == 'c') {
            index--;
          } else {
            usr_psw[index] = key;
            index++;
          }
        }

        if (index == (sizeof(psw) / sizeof(char)) - 1) {
          if (psw_check(psw, usr_psw, index)) {
            index = 0;
            adesso = millis();
            state = 2;
          } else {
            res();

            lcd.clear();
            stamp("Psw  Errata!", 0);

            delay(1000);
          }
        } else {
          for (int i = 0; i < index; i++) {
            lcd.setCursor(6 + i, 1);
            lcd.print(usr_psw[i]);
          }
        }
      }
      break;

    case 2: {
        // Limite massimo di tempo impostabile 20 giorni, altrimenti questa variabile diventa negativa e il programma smette di funzionare
        long time_left = max_time - (millis() - adesso);

        if (time_left > 0) {
          if (!final_psw || (millis() - last_time_press) > 2000) {
            stamp("Tempo  Rimasto", 0);
            time_stamp(time_left);
          } else {
            stamp("Inserisci  Psw", 0);

            for (int i = 0; i < index; i++) {
              lcd.setCursor(6 + i, 1);
              lcd.print(usr_psw[i]);
            }
          }

          char key = pad.getKey();

          if (final_psw) {
            if (key != NO_KEY) {
              last_time_press = millis();
              if (key == 'c' && index > 0) {
                index--;
              } else if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9') {
                if (index < (sizeof(psw) / sizeof(char)) - 1) {
                  usr_psw[index] = key;
                  index++;
                }
              } else if (key == 'a') {
                if (index == (sizeof(psw) / sizeof(char)) - 1) {
                  res();

                  if (psw_check(psw, usr_psw, index)) {
                    lcd.clear();
                    stamp("Disinnescata", 0);
                    delay(2000);
                  } else {
                    lcd.clear();
                    stamp("BOOOOM", 0);
                    delay(2000);
                  }
                }
              }
            }
          } else {
            if (key == 'a') {
              res();

              lcd.clear();
              stamp("Disinnescata", 0);
              delay(2000);
            }
          }
        } else {
          res();

          lcd.clear();
          stamp("BOOOOM", 0);
          delay(2000);
        }
      }
      break;

    case 5: {
        stamp("s_usr  psw", 0);
        psw_stamp(usr_psw, index, (sizeof(super_user_psw) / sizeof(char)) - 1);

        char key = pad.getKey();

        if (key != NO_KEY) {
          if (key == 'c' && index > 0) {
            index--;
          } else if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9') {
            if (index < (sizeof(super_user_psw) / sizeof(char)) - 1) {
              usr_psw[index] = key;
              index++;
            }
          } else if (key == 'a') {
            if (index == (sizeof(super_user_psw) / sizeof(char)) - 1) {
              if (psw_check(psw, usr_psw, index)) {
                lcd.clear();
                state = 6;
                stamp("Psw Accettata!", 0);
                delay(1000);
              } else {
                lcd.clear();
                state = 0;
                stamp("Psw Respinta", 0);
                delay(1000);
              }

              index = 0;
            }
          }
        }
      }
      break;

    case 6: {
        switch (menu) {
          case 0: { // password iniziale
              stamp("psw iniziale", 0);
              lcd.setCursor(16, 0);
              lcd.print(byte(0));

              if (initial_psw)
                stamp("attivata", 1);
              else
                stamp("disattivata", 1);

              lcd.setCursor(16, 1);
              lcd.print(1);

              char key = pad.getKey();

              if (key != NO_KEY) {
                if (key == "c")
                  menu = 4;
                else if (key == "d")
                  menu++;

              }
            }
            break;

          case 1: { // password finale
              stamp("psw finale", 0);
              lcd.setCursor(16, 0);
              lcd.print(byte(0));

              if (final_psw)
                stamp("attivata", 1);
              else
                stamp("disattivata", 1);

              lcd.setCursor(16, 1);
              lcd.print(1);

              char key = pad.getKey();

              if (key != NO_KEY) {
                if (key == "c")
                  menu--;
                else if (key == "d") {
                  if (final_psw || initial_psw)
                    menu++;
                  else
                    menu += 2;
                }

              }
            }
            break;

          case 2: { // password (se iniziale o finale)
              stamp("modifica psw", 0);
              lcd.setCursor(16, 0);
              lcd.print(byte(0));

              lcd.setCursor(16, 1);
              lcd.print(1);

              char key = pad.getKey();

              if (key != NO_KEY) {
                if (key == "c")
                  menu--;
                else if (key == "d")
                  menu++;

              }
            }
            break;

          case 3: { //durata partita
              stamp("durata partita", 0);
              lcd.setCursor(16, 0);
              lcd.print(byte(0));

              lcd.setCursor(16, 1);
              lcd.print(1);

              char key = pad.getKey();

              if (key != NO_KEY) {
                if (key == "c")
                  menu--;
                else if (key == "d")
                  menu++;

              }
            }
            break;

          case 4: { // password super user
              stamp("modifica psw", 0);
              lcd.setCursor(16, 0);
              lcd.print(byte(0));

              stamp("amministratore", 1);
              lcd.setCursor(16, 1);
              lcd.print(1);

              char key = pad.getKey();

              if (key != NO_KEY) {
                if (key == "c")
                  menu--;
                else if (key == "d")
                  menu = 0;

              }
            }
            break;
        }
      }
      break;
  }

  delay(50);
}

bool psw_check(char* psw1, char* psw2, int dim) {
  for (int i = 0; i < dim; i++) {
    if (psw1[i] != psw2[i]) {
      return false;
    }
  }
  return true;
}

void psw_stamp(char *psw, int dim_psw, int final_psw_dim) {
  for (int i = 0; i < final_psw_dim; i++) {
    lcd.setCursor(8 - (final_psw_dim / 2) + i, 1);
    if (i < dim_psw)
      lcd.print(psw[i]);
    else
      lcd.print('_');
  }
}

void res() {
  state = 0;
  index = 0;
}

void stamp(String str, int row) {
  int len = str.length();
  lcd.setCursor(8 - (len / 2), row);

  lcd.print(str);
}

void time_stamp(long time_left) {
  long second_left = time_left / 1000;

  long ms = time_left % 1000;
  ms = ms / 10;
  long sec = second_left % 60;
  long minuti = second_left / 60;

  lcd.setCursor(2, 1);
  lcd.print(minuti / 10);

  lcd.setCursor(3, 1);
  lcd.print(minuti % 10);

  lcd.setCursor(4, 1);
  lcd.print(" : ");

  lcd.setCursor(7, 1);
  lcd.print(sec / 10);

  lcd.setCursor(8, 1);
  lcd.print(sec % 10);

  lcd.setCursor(9, 1);
  lcd.print(" : ");

  lcd.setCursor(12, 1);
  lcd.print(ms / 10);

  lcd.setCursor(13, 1);
  lcd.print(ms % 10);
}

void keypadEvent(KeypadEvent key) {
  if (pad.getState() == HOLD && state == 0) {
    if (key == '#') {
      state = 5;
    }
  }
}
