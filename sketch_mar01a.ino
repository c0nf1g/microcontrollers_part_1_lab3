#include <LiquidCrystal.h>

#define DDR_KEYPAD DDRA
#define PORT_KEYPAD PORTA
#define PIN_KEYPAD PINA

#include "keypad4x4.h"

const int rs = 54,
  rw = 55,
  en = 56,
  d4 = 57,
  d5 = 58,
  d6 = 59,
  d7 = 60;
const int buzzer = 21;
char button;
char input[4];
int counter = 0;
int mode;

char check[3];
int checkHour;
int checkMin;

bool allowed = true;
int clickedC = 0;
int clickedD = 0;
int startedOne, startedSecond;
int startBuzzing = 0;

LiquidCrystal lcd(rs, rw, en, d4, d5, d6, d7);

const PROGMEM  char sixty[60][3] = {
  {"00"}, {"01"}, {"02"}, {"03"}, {"04"}, {"05"}, {"06"}, {"07"}, {"08"}, {"09"},
  {"10"}, {"11"}, {"12"}, {"13"}, {"14"}, {"15"}, {"16"}, {"17"}, {"18"}, {"19"},
  {"20"}, {"21"}, {"22"}, {"23"}, {"24"}, {"25"}, {"26"}, {"27"}, {"28"}, {"29"},
  {"30"}, {"31"}, {"32"}, {"33"}, {"34"}, {"35"}, {"36"}, {"37"}, {"38"}, {"39"},
  {"40"}, {"41"}, {"42"}, {"43"}, {"44"}, {"45"}, {"46"}, {"47"}, {"48"}, {"49"},
  {"50"}, {"51"}, {"52"}, {"53"}, {"54"}, {"55"}, {"56"}, {"57"}, {"58"}, {"59"}
};

struct TimerOne {
  unsigned char hour, minute, second;
};

struct TimerTwo {
  unsigned char hour, minute, second;
};

TimerOne timerOne = {
  0,
  0,
  0
};
TimerTwo timerTwo = {
  0,
  0,
  0
};

void LCD_WriteStrPROGMEM(char * str, int n) {
  for (int i = 0; i < n; i++)
    lcd.print((char) pgm_read_byte( & (str[i])));
}

ISR(TIMER1_COMPA_vect) // Таймер Т1 по співпадінню А, кожної 1 сек.
{
  if (clickedC == 1) {
    if (++timerOne.second == 60) {
      timerOne.second = 0;
      if (timerOne.minute != 0) {
        timerOne.minute--;
      } else if (timerOne.minute == 0) {
        timerOne.minute = 0;

        if (timerOne.hour != 0) {
          timerOne.hour--;
        } else if (timerOne.hour == 0) {
          startBuzzing = 1;
          timerOne.hour = 0;
          clickedC = 0;
          clearLED(0);
        }
      }
    }

    lcd.setCursor(4, 0);
    LCD_WriteStrPROGMEM(sixty[timerOne.hour], 2);
    lcd.write(':');
    LCD_WriteStrPROGMEM(sixty[timerOne.minute], 2);
    lcd.write(':');
    if (timerOne.second == 0) {
      LCD_WriteStrPROGMEM(sixty[0], 2);
    } else {
      LCD_WriteStrPROGMEM(sixty[60 - timerOne.second], 2);
    }
  }
  if (clickedD == 1) {
    if (++timerTwo.second == 60) {
      timerTwo.second = 0;
      if (timerTwo.minute != 0) {
        timerTwo.minute--;
      } else if (timerTwo.minute == 0) {
        timerTwo.minute = 0;

        if (timerTwo.hour != 0) {
          timerTwo.hour--;
        } else if (timerTwo.hour == 0) {
          startBuzzing = 1;
          timerTwo.hour = 0;
          clickedD = 0;
          clearLED(1);
        }
      }
    }

    lcd.setCursor(4, 1);
    LCD_WriteStrPROGMEM(sixty[timerTwo.hour], 2);
    lcd.write(':');
    LCD_WriteStrPROGMEM(sixty[timerTwo.minute], 2);
    lcd.write(':');
    if (timerTwo.second == 0) {
      LCD_WriteStrPROGMEM(sixty[0], 2);
    } else {
      LCD_WriteStrPROGMEM(sixty[60 - timerTwo.second], 2);
    }
  }

  if (startBuzzing != 0) {
      if (++startBuzzing % 2 == 0) {
        digitalWrite(buzzer, HIGH);
      } else if (startBuzzing == 5) {
        startBuzzing = 0;
        digitalWrite(buzzer, LOW);
      } else {
        digitalWrite(buzzer, LOW);
      }
    }
}

void setup() {
  noInterrupts();

  TCCR1A = 0x00;
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
  TIMSK1 = (1 << OCIE1A);
  OCR1A = 0x3D08;

  initKeyPad();
  lcd.begin(16, 2);

  interrupts();

  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);

  lcd.setCursor(4, 0);
  LCD_WriteStrPROGMEM(sixty[0], 2);
  lcd.write(':');
  LCD_WriteStrPROGMEM(sixty[0], 2);
  lcd.write(':');
  LCD_WriteStrPROGMEM(sixty[0], 2);

  lcd.setCursor(4, 1);
  LCD_WriteStrPROGMEM(sixty[0], 2);
  lcd.write(':');
  LCD_WriteStrPROGMEM(sixty[0], 2);
  lcd.write(':');
  LCD_WriteStrPROGMEM(sixty[0], 2);
}

bool checkTime(char * input) {
  check[0] = input[0];
  check[1] = input[1];
  sscanf(check, "%d", & checkHour);

  check[0] = input[2];
  check[1] = input[3];
  sscanf(check, "%d", & checkMin);

  return (checkHour <= 24 && checkMin <= 59 && ((checkHour != 0 && checkMin == 0) || (checkHour == 0 && checkMin != 0) || (checkHour != 0 && checkMin != 0)));
}

void clearLED(int timer) {
  if (timer == 0) {
    timerOne.second = 0;
    timerOne.minute = 0;
    timerOne.hour = 0;
  } else if (timer == 1) {
    timerTwo.second = 0;
    timerTwo.minute = 0;
    timerTwo.hour = 0;
  }

  lcd.setCursor(4, timer);
  LCD_WriteStrPROGMEM(sixty[0], 2);
  lcd.write(':');
  LCD_WriteStrPROGMEM(sixty[0], 2);
  lcd.write(':');
  LCD_WriteStrPROGMEM(sixty[0], 2);
}

void loop() {
  if (isButtonPressed()) {
    digitalWrite(buzzer, HIGH);
    button = readKeyFromPad4x4();
    delay(100);
    digitalWrite(buzzer, LOW);

    if (button == 'A') {
      mode = 0;
      counter = 0;
      for (int i = 0; i < 4; i++) {
        input[i] = 0;
      }
    } else if (button == 'B') {
      mode = 1;
      counter = 0;
      for (int i = 0; i < 4; i++) {
        input[i] = 0;
      }
    } else if (mode == 0) {
      input[counter++] = button;
      lcd.setCursor(4, 0);
      lcd.write(input[0]);
      lcd.write(input[1]);
      lcd.write(':');
      lcd.write(input[2]);
      lcd.write(input[3]);
    } else if (mode == 1) {
      input[counter++] = button;
      lcd.setCursor(4, 1);
      lcd.write(input[0]);
      lcd.write(input[1]);
      lcd.write(':');
      lcd.write(input[2]);
      lcd.write(input[3]);
    }
    if (button == 'E') {
      if (checkTime(input)) {
        if (mode == 0) {
          timerOne.hour = checkHour;
          timerOne.minute = checkMin;
          checkHour = 0;
          checkMin = 0;
        } else if (mode == 1) {
          timerTwo.hour = checkHour;
          timerTwo.minute = checkMin;
          checkHour = 0;
          checkMin = 0;
        }
      } else {
        allowed = false;
        clearLED(mode);
        digitalWrite(buzzer, HIGH);
        delay(300);
        digitalWrite(buzzer, LOW);
      }
    }
    if (button == 'C' && allowed) {
      if (clickedC == 0) {
        clickedC++;
        counter = 0;

        if (timerOne.hour == 0 && timerOne.minute != 0) {
          timerOne.minute--;
        } else if (timerOne.hour != 0 && timerOne.minute == 0) {
          timerOne.hour--;
          timerOne.minute = 59;
        } else {
          timerOne.minute--;
        }
      } else if (clickedC == 1) {
        clickedC = 0;
        clearLED(0);
      }
    }
    if (button == 'D' && allowed) {
      if (clickedD == 0) {
        clickedD++;
        counter = 0;

        if (timerTwo.hour == 0 && timerTwo.minute != 0) {
          timerTwo.minute--;
        } else if (timerTwo.hour != 0 && timerTwo.minute == 0) {
          timerTwo.hour--;
          timerTwo.minute = 59;
        } else {
          timerTwo.minute--;
        }
      } else if (clickedD == 1) {
        clickedD = 0;
        clearLED(1);
      }
    } else if (button == 'F') {
      clearLED(mode);
    }
  }
}
