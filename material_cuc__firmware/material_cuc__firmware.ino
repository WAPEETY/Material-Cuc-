#include <Elegoo_TFTLCD.h>
#include <Elegoo_GFX.h>
#include <TouchScreen.h>
#include <Wire.h>
#include <RTClib.h>

#define LCD_RESET 10 //a 4 pin che usa lo schermo
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1 //a1
#define LCD_RD A0 //a0

#define TS_MINX 927  //calibrazione touch/LCD
#define TS_MINY 899
#define TS_MAXX 128
#define TS_MAXY 130

#define YP A3  //pin usati dal touch
#define XM A2
#define YM 9
#define XP 8

#define BLACK 0x0000 //colori in formato RGB565 (converter: http://www.barth-dev.de/online/rgb565-color-picker/) in ogni caso sono 5 bit per il rosso, 6 per il verde e 5 per il blu
#define WHITE 0xFFFF
#define LIGHT_GRAY 0xC618
#define INDIGO  0x3A96

#define MAX_LENGTH_TITLE 46

#define rxPin 0
#define txPin 1

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET); //comunicazione pin-libreria
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 364);

RTC_DS1307 rtc;

unsigned long time;
unsigned long secondi;
unsigned long minuti;
unsigned long ore;
unsigned long giorni;
char c = 'N';
String comando = "SALVAMI SULL'SD, CESSO<";
bool unix = false;

/* this array contains the received data from the bluetooth socket */
const char custom_title[MAX_LENGTH_TITLE] = "\0";

struct alarm {
	/* this record keeps clean the alarms */
	int hour;
	int minute;
};

struct alarm_holder {
	alarm alarm0;
	alarm alarm1;
	alarm alarm2;
} *alarms;

struct _current_time {
	/* used only once, but still makes the code more readable */
	int hour;
	int minute;
	int second;
} *current_time;

void setup(void){
  Serial.begin(9600);
  Wire.begin();
  if (!rtc.begin()){
    Serial.println("ERRORE, Ho riscontrato problemi nell'inizializzare una comunicazione con l'RTC");
  }
  if (!rtc.isrunning()){
    Serial.println("il chip non ha le informazioni di data e ora, lo sto settando secondo il compilatore, se credi sia sbagliato sincronizza l'app");
    rtc.adjust(DateTime(__DATE__, __TIME__));
    }
  tft.reset();
  
  uint16_t identifier = tft.readID();
  if(identifier == 0x9325) {
    Serial.println(F("Trovati driver dell'LCD ILI9325"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Trovati driver dell'LCD ILI9328"));
  } else if(identifier == 0x4535) {
    Serial.println(F("Trovati driver dell'LCD LGDP4535"));
  }else if(identifier == 0x7575) {
    Serial.println(F("Trovati driver dell'LCD HX8347G"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Trovati driver dell'LCD ILI9341"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Trovati driver dell'LCD HX8357D"));
  } else if(identifier==0x0101)
  {     
      identifier=0x9341;
       Serial.println(F("Found 0x9341 LCD driver"));
  }else {
    Serial.print(F("Driver non trovati per: "));
    Serial.println(identifier, HEX);
    Serial.println(F("Se stai usando Elegoo 2.8\" TFT Arduino shield, la riga:"));
    Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
    Serial.println(F("dovrebbe apparire all'inizio della libreria (Elegoo_TFT.h)."));
    identifier=0x9341;
   
  }

  tft.begin(identifier);
  tft.setRotation(1);

  tft.fillScreen(WHITE);
  tft.fillRect(0, 0, 330, 40, INDIGO);
  tft.fillRect(10, 60, 300, 80, LIGHT_GRAY);
  tft.fillRect(10, 160, 300, 70, LIGHT_GRAY);
  tft.setTextColor(INDIGO, WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 44);
  tft.print("NEXT ALARM");
  tft.setCursor(10, 144);
  tft.print("CUSTOM MESSAGE");
  tft.setCursor(160, 13);
  tft.setTextSize(2);
  tft.setTextColor(WHITE, INDIGO);
  tft.print("NOT CONNECTED");
}

void loop() {
  DateTime now = rtc.now();
    if(Serial.available()) {
			/* the data is received as such:
			 * [ byte_current_hour][byte_current_minute][byte_current_second]
			 * [ byte_alarm0_hour ][byte_alarm0_minute ][byte_alarm1_hour   ]
			 * [byte_alarm1_minute][byte_alarm2_hour   ][byte_alarm2_minute ]
			 */

			/* get the current time */
			char current_byte;
			memset(custom_title, '\0', sizeof(custom_title));
			for (int i = 0; i < 3; i++) {
				/* read the current time */
				current_byte = Serial.read();
				*current_time = ((int) current_byte);
				current_time++;
			}

			/* get the alarms */
			for (int i = 0; i < 3; i++) {
				current_byte = Serial.read();
				*alarms.hour = ((int) current_byte);
				current_byte = Serial.read();
				*alarms.minute = ((int) current_byte);
				alarm_holder += sizeof (alarm);
			}

			current_byte = 'x';
			
			for (int i = 0; i < MAX_LENGTH_TITLE && current_byte != '\0'; i++) {
				current_byte = Serial.read();
				custom_title[i] = current_byte;
			}
    }
  tft.setCursor(10,10);
  if (unix == false){
  tft.setTextSize(3);
  tft.setTextColor(WHITE, INDIGO);
  if (now.hour() < 10){
    tft.print("0");
  }
  tft.print(now.hour(), DEC);
  tft.print(":");
  if (now.minute() < 10){
    tft.print("0");
  }
  tft.print(now.minute(), DEC);
  tft.print(":");
  if (now.second() < 10){
    tft.print("0");
  }
  tft.print(now.second(), DEC);
  }
  else{
  tft.setTextSize(2);
  tft.print(now.unixtime(), DEC);
  }
  tft.setCursor(160, 13);
  tft.setTextSize(2);
  if (comando == "SI<")
    tft.print("    CONNECTED");
  else if (comando == "NO<"){
    tft.print("NOT CONNECTED");
	}
  tft.setTextColor(WHITE, LIGHT_GRAY);
  tft.setTextSize(7);
  tft.setCursor(60, 75);
  tft.print("00:00");
  tft.setTextSize(2);
  tft.setCursor(20, 190);
  tft.print(comando);
  
//  TSPoint p = ts.getPoint();
//  if (p.z > ts.pressureThreshhold) {
//    p.x = map(p.x, TS_MAXX, TS_MINX, 0, 320);
//    p.y = map(p.y, TS_MAXY, TS_MINY, 0, 480);
//    if(p.x > 10 && p.x < 150 && p.y > 10 && p.y < 40){
//      if (unix == false)
//        unix = true;
//      else
//        unix = false;
//    }
//  }
}
