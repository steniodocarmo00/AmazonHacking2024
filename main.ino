#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

#define endereco  0x27
#define colunas   16
#define linhas    2

LiquidCrystal_I2C lcd(endereco, colunas, linhas); 

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))
#include <SoftwareSerial.h>
SoftwareSerial softSerial(/rx =/26, /tx =/27);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif
DFRobotDFPlayerMini player;

#define SS_PIN 5
#define RST_PIN 4
String tagID = "";
bool acessoliberado = false;


String TagsLiberadas[] = {"2b7f3ba", "ID2"}; 
MFRC522 RFID(SS_PIN, RST_PIN);

void setup()
{
  #if (defined ESP32)
    FPSerial.begin(9600, SERIAL_8N1, /rx =/26, /tx =/27);
  #else
    FPSerial.begin(9600);
  #endif

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!player.begin(FPSerial, true, true)) { 
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  Serial.println(F("DFPlayer Mini online."));

  lcd.init();         
  lcd.backlight();    
  SPI.begin();        
  RFID.PCD_Init();    
  lcd.print(" Pira__  ");

  player.volume(30);
  player.play(3);
}

void loop() {

if (!RFID.PICC_IsNewCardPresent() || !RFID.PICC_ReadCardSerial() ) {
  return;
  }

  tagID = "";


  for (byte i = 0; i < RFID.uid.size; i++) {
    tagID.concat(String(RFID.uid.uidByte[i], HEX));
  }


  for (int i = 0; i < (sizeof(TagsLiberadas)/sizeof(String)); i++) {
    if (  tagID.equalsIgnoreCase(TagsLiberadas[i])  ) {
      acessoliberado = true; 
    }
  }
  if (acessoliberado == true) {
    Liberado();
    lcd.clear();
    lcd.print(" Pirarucu  ");
    player.play(4);
    delay(3000);

  } 
    else {
    Negado();
    lcd.clear();
    lcd.print("Errado!");
    player.play(5);
    delay(3000);
  }
  delay(1000);
}

void Liberado() {
  int avisos = 1;
  for (int j = 0; j < avisos; j++) {
    delay(3000);
    delay(1000);
  }
  acessoliberado = false;
}

void Negado() {
  int avisos = 2;
  for (int j = 0; j < avisos; j++) {
    delay(3000);
    delay(1000);
  }
}
