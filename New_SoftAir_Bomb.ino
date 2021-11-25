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

Keypad pad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

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

char count_down[7];
unsigned long adesso;
unsigned long max_time;
unsigned long last_time_press;

int state;

bool initial_psw;
bool final_psw;

char psw[5];
char usr_psw[5];

char super_user_psw[5];

int index;

int menu;
bool change;

bool new_initial_psw;
bool new_final_psw;
unsigned long new_max_time;
int new_max_time_sup[7];
char new_psw[5];
char new_super_user_psw[5];

void setup() {
  EEPROM.get(0, initial_psw);
  EEPROM.get(1, final_psw);

  EEPROM.get(2, psw); // occupa i Byte da 2 a 7
  EEPROM.get(8, super_user_psw); // occupa i Byte da 8 a 13

  EEPROM.get(14, count_down); // occupa i Byte da 14 a 21

  int count_down_sup[6];

  for (int i = 0; i < 6; i++)
    count_down_sup[i] = count_down[i] - '0';

  max_time = from_array_to_time(count_down_sup);

  pad.setHoldTime(500);
  pad.setDebounceTime(100);
  pad.addEventListener(keypadEvent);

  last_time_press = 5000;
  menu = 0;
  change = false;
  res();
  lcd.begin(16, 2);

  lcd.createChar(0, up);
  lcd.createChar(1, down);

  new_initial_psw = initial_psw;
  new_final_psw = final_psw;
  new_max_time = max_time;

  for (int i = 0; i < (sizeof(psw) / sizeof(char)) - 1; i++)
    new_psw[i] = psw[i];

  for (int i = 0; i < (sizeof(super_user_psw) / sizeof(char)) - 1; i++)
    new_super_user_psw[i] = super_user_psw[i];
}

void loop() {
  lcd.clear();

  switch (state) {
    case 0:
      if (initial_psw) {
        stamp("Inserire  Psw", 0);
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
        stamp("Inserire  Psw", 0);
        psw_stamp(usr_psw, index, (sizeof(psw) / sizeof(char)) - 1);

        char key = pad.getKey();

        if (key != NO_KEY) {
          if (key == 'c') {
            index--;
          } else if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9') {
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
        }
      }
      break;

    case 2: {
        // Limite massimo di tempo impostabile circa 20 giorni
        long time_left = max_time - (millis() - adesso);

        if (time_left > 0) {
          if (!final_psw || (millis() - last_time_press) > 2000) {
            stamp("Tempo  Rimasto", 0);
            time_stamp(time_left);
          } else {
            stamp("Inserisci  Psw", 0);
            psw_stamp(usr_psw, index, (sizeof(psw) / sizeof(char)) - 1);
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
                  if (psw_check(psw, usr_psw, index)) {
                    lcd.clear();
                    stamp("Disinnescata", 0);
                    delay(2000);
                  } else {
                    lcd.clear();
                    stamp("BOOOOM", 0);
                    delay(2000);
                  }
                  res();
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
              if (psw_check(super_user_psw, usr_psw, index)) {
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
          } else if (key == 'b')
            res();
        }
      }
      break;

    case 6: {
        switch (menu) {
          case 0: { // password iniziale
              if (!change) {
                stamp("psw iniziale", 0);
                lcd.setCursor(16, 0);
                lcd.print(byte(0));

                if (new_initial_psw)
                  stamp("attivata", 1);
                else
                  stamp("disattivata", 1);

                lcd.setCursor(16, 1);
                lcd.print(1);

                char key = pad.getKey();
                if (key != NO_KEY) {
                  if (key == 'c')
                    menu = 4;
                  else if (key == 'd')
                    menu++;
                  else if (key == 'a')
                    change = !change;
                  else if (key == 'b')
                    menu = 5;
                }
              } else {
                if (new_initial_psw)
                  stamp("disattivare?", 0);
                else
                  stamp("attivare?", 0);
                lcd.setCursor(16, 0);
                lcd.print(byte(0));

                stamp("A: Si  B: No", 1);
                lcd.setCursor(16, 1);
                lcd.print(1);

                char key = pad.getKey();
                if (key != NO_KEY) {
                  if (key == 'a') {
                    new_initial_psw = !new_initial_psw;
                    change = !change;
                  } else if (key == 'b')
                    change = !change;
                }
              }
            }
            break;

          case 1: { // password finale
              if (!change) {
                stamp("psw finale", 0);
                lcd.setCursor(16, 0);
                lcd.print(byte(0));

                if (new_final_psw)
                  stamp("attivata", 1);
                else
                  stamp("disattivata", 1);

                lcd.setCursor(16, 1);
                lcd.print(1);

                char key = pad.getKey();

                if (key != NO_KEY) {
                  if (key == 'c')
                    menu--;
                  else if (key == 'd') {
                    //if (final_psw || initial_psw)
                    menu++;
                    //else
                    // menu += 2;
                  } else if (key == 'a')
                    change = !change;
                  else if (key == 'b')
                    menu = 5;
                }
              } else {
                if (new_final_psw)
                  stamp("disattivare?", 0);
                else
                  stamp("attivare?", 0);
                lcd.setCursor(16, 0);
                lcd.print(byte(0));

                stamp("A: Si  B: No", 1);
                lcd.setCursor(16, 1);
                lcd.print(1);

                char key = pad.getKey();
                if (key != NO_KEY) {
                  if (key == 'a') {
                    new_final_psw = !new_final_psw;
                    change = !change;
                    lcd.clear();
                  } else if (key == 'b')
                    change = !change;
                }
              }
            }
            break;

          case 2: { // password (se iniziale o finale)
              if (!change) {
                //stamp("modifica psw", 0);
                stamp("psw  bomba", 0);
                lcd.setCursor(16, 0);
                lcd.print(byte(0));

                stamp(psw, 1);
                lcd.setCursor(16, 1);
                lcd.print(1);

                char key = pad.getKey();

                if (key != NO_KEY) {
                  if (key == 'a')
                    change = !change;
                  else if (key == 'c')
                    menu--;
                  else if (key == 'd')
                    menu++;
                  else if (key == 'b')
                    menu = 5;
                }
              } else {
                stamp("nuova  psw", 0);
                lcd.setCursor(16, 0);
                lcd.print(byte(0));

                psw_stamp(new_psw, index, (sizeof(psw) / sizeof(char)) - 1);
                lcd.setCursor(16, 1);
                lcd.print(1);

                char key = pad.getKey();

                if (key != NO_KEY) {
                  if (key == 'b') {
                    change = !change;
                    index = 0;
                    for (int i = 0; i < (sizeof(psw) / sizeof(char)) - 1; i++) {
                      new_psw[i] = psw[i];
                    }
                  } else if (key == 'a' && index == (sizeof(psw) / sizeof(char)) - 1) {
                    change = !change;
                    index = 0;
                  }
                  else if (key == 'c' && index > 0)
                    index--;
                  else if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9') {
                    if (index < (sizeof(psw) / sizeof(char)) - 1) {
                      new_psw[index] = key;
                      index++;
                    }
                  }
                }
              }
            }
            break;

          case 3: { //durata partita
              if (!change) {
                stamp("durata partita", 0);
                lcd.setCursor(16, 0);
                lcd.print(byte(0));

                time_stamp(new_max_time);
                lcd.setCursor(16, 1);
                lcd.print(1);

                char key = pad.getKey();

                if (key != NO_KEY) {
                  if (key == 'a')
                    change = !change;
                  else if (key == 'c')
                    menu--;
                  else if (key == 'd')
                    menu++;
                  else if (key == 'b')
                    menu = 5;
                }
              } else {
                stamp("nuova durata", 0);
                lcd.setCursor(16, 0);
                lcd.print(byte(0));

                new_time_stamp(new_max_time_sup, index);
                lcd.setCursor(16, 1);
                lcd.print(1);

                char key = pad.getKey();

                if (key != NO_KEY) {
                  if (key == 'b') {
                    change = !change;
                    index = 0;
                  } else if (key == 'a' && index == 6) {
                    change = !change;
                    index = 0;
                    new_max_time = from_array_to_time(new_max_time_sup);
                  }
                  else if (key == 'c' && index > 0)
                    index--;
                  else if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9') {
                    if (index < 6) {
                      new_max_time_sup[index] = key - '0';
                      index++;
                    }
                  }
                }
              }
            }
            break;

          case 4: { // password super user
              if (!change) {
                //stamp("modifica psw", 0);
                stamp("s_usr  psw", 0);
                lcd.setCursor(16, 0);
                lcd.print(byte(0));

                // stamp("amministratore", 1);
                stamp(super_user_psw, 1);
                lcd.setCursor(16, 1);
                lcd.print(1);

                char key = pad.getKey();

                if (key != NO_KEY) {
                  if (key == 'a')
                    change = !change;
                  else if (key == 'c')
                    menu--;
                  else if (key == 'd')
                    menu = 0;
                  else if (key == 'b')
                    menu = 5;
                }
              } else {
                stamp("nuova  psw", 0);
                lcd.setCursor(16, 0);
                lcd.print(byte(0));

                psw_stamp(new_super_user_psw, index, (sizeof(super_user_psw) / sizeof(char)) - 1);
                lcd.setCursor(16, 1 );
                lcd.print(1);

                char key = pad.getKey();

                if (key != NO_KEY) {
                  if (key == 'b') {
                    change = !change;
                    index = 0;
                    for (int i = 0; i < (sizeof(super_user_psw) / sizeof(char)) - 1; i++) {
                      new_super_user_psw[i] = super_user_psw[i];
                    }
                  } else if (key == 'a' && index == (sizeof(super_user_psw) / sizeof(char)) - 1) {
                    change = !change;
                    index = 0;
                  }
                  else if (key == 'c' && index > 0)
                    index--;
                  else if (key == '0' || key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9') {
                    if (index < (sizeof(super_user_psw) / sizeof(char)) - 1) {
                      new_super_user_psw[index] = key;
                      index++;
                    }
                  }
                }
              }
            }
            break;

          case 5: {
              stamp("salvare?", 0);
              lcd.setCursor(16, 0);
              lcd.print(byte(0));

              stamp("A: Si  B: No", 1);
              lcd.setCursor(16, 1);
              lcd.print(1);

              char key = pad.getKey();

              if (key != NO_KEY) {
                if (key == 'a') {
                  initial_psw = new_initial_psw;
                  final_psw = new_final_psw;
                  max_time = new_max_time;

                  for (int i = 0; i < (sizeof(new_psw) / sizeof(char)) - 1; i++) {
                    psw[i] = new_psw[i];
                  }

                  for (int i = 0; i < (sizeof(new_super_user_psw) / sizeof(char)) - 1; i++) {
                    super_user_psw[i] = new_super_user_psw[i];
                  }

                  EEPROM.update(0, new_initial_psw);
                  EEPROM.update(1, new_final_psw);

                  for (int i = 0; i < 4; i++)
                    EEPROM.update(2 + i, new_psw[i]);

                  for (int i = 0; i < 4; i++)
                    EEPROM.update(8 + i, new_super_user_psw[i]);

                  for (int i = 0; i < 6; i++)
                    EEPROM.update(14 + i * sizeof(char), (char)(new_max_time_sup[i] + '0'));

                  lcd.clear();

                  stamp("impostazioni", 0);
                  lcd.setCursor(16, 0);
                  lcd.print(byte(0));

                  stamp("salvate!", 1);
                  lcd.setCursor(16, 1);
                  lcd.print(1);

                  delay(1000);

                  menu = 0;
                  res();
                } else if (key == 'b') {
                  new_initial_psw = initial_psw;
                  new_final_psw = final_psw;
                  new_max_time = max_time;

                  menu = 0;
                  res();
                }
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

void new_time_stamp(int* new_time, int index) {
  lcd.setCursor(4, 1);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      if (index > (i * 2 + j)) {
        lcd.print(new_time[i * 2 + j]);
      } else {
        lcd.print("_");
      }
    }

    if (i != 2) {
      lcd.print(":");
    }
  }
}

unsigned long from_array_to_time(int* new_time) {
  unsigned long count = 0;

  count += (unsigned long)new_time[0] * 10 * 60 * 1000;
  count += (unsigned long)new_time[1] * 60 * 1000;
  count += (unsigned long)new_time[2] * 10 * 1000;
  count += (unsigned long)new_time[3] * 1000;
  count += (unsigned long)new_time[4] * 100;
  count += (unsigned long)new_time[5] * 10;

  return count;
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
