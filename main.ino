#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

#define endereco  0x27
#define colunas   16
#define linhas    2

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))
#include <SoftwareSerial.h>
SoftwareSerial softSerial(26, 27);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif

#define SS_PIN 5
#define RST_PIN 4

LiquidCrystal_I2C lcd(endereco, colunas, linhas); 

DFRobotDFPlayerMini player;

MFRC522 RFID(SS_PIN, RST_PIN);

class WordTag {
  public: 
  String tag;
  String wordStr;
  String unfinishedWord;
  int tries;
  String FindWordByTag(String tag);
  String FindUnfinishedWordByTag(String tag);
  void AnswerValidation();

WordTag(String t, String w, String u) : tag(t), wordStr(w), unfinishedWord(u), tries(0) {}

};

int randomNumber;
String randomWord;
String randomTag;
String unfinishedRandomWord;
String tagID = "";
bool gameEnded = false;

WordTag tp[] = {
  {"3b891f3", "jambu", "___bu"}, 
  {"9e6c2f93", "bacuri", "ba____"}, 
  {"ac24213", "castanha", "cas_____"}, 
  {"a57b1d3", "jenipapo", "____papo"}, 
  {"dba71c3", "pirarucu", "pira____"}, 
  {"89d61e3", "pupunha", "____nha"}, 
  {"71701e3", "dourada", "dou____"}, 
  {"2cf1d3", "tambaqui", "_____qui"},
};

int playerScore = 0;
int currentRound = 0;
const int maxRounds = 3;

String WordTag::FindWordByTag(String tag) {
  for(int i = 0; i < 8; i++) {
    if(tp[i].tag == tag) {
      return tp[i].wordStr;
    }
  }
  return "";
};

String WordTag::FindUnfinishedWordByTag(String tag) {
  for(int i = 0; i < 8; i++) {
    if(tp[i].tag == tag) {
      return tp[i].unfinishedWord;
    }
  }
  return "";
};

void(* resetFunc) (void) = 0;

void StartGame() {
  randomNumber = random(8);
  randomTag = tp[randomNumber].tag;
  randomWord = tp[randomNumber].FindWordByTag(randomTag);
  unfinishedRandomWord = tp[randomNumber].FindUnfinishedWordByTag(randomTag);
  gameEnded = false;

  switch (randomNumber) {
    case 0:
      lcd.clear();
      lcd.print(unfinishedRandomWord);
      //player.play(1);
      delay(1500);
      tp[randomNumber].AnswerValidation();
      break;
    case 1:
      lcd.clear();
      lcd.print(unfinishedRandomWord);
      //player.play(2);
      delay(1500);
      tp[randomNumber].AnswerValidation();
      break;
    case 2:
      lcd.clear();
      lcd.print(unfinishedRandomWord);
     // player.play(3);
      delay(1500);
      tp[randomNumber].AnswerValidation();
      break;
    case 3:
      lcd.clear();
      lcd.print(unfinishedRandomWord);
    //  player.play(4);
      delay(1500);
      tp[randomNumber].AnswerValidation();
      break;
    case 4:
      lcd.clear();
      lcd.print(unfinishedRandomWord);
   //   player.play(5);
      delay(1500);
      tp[randomNumber].AnswerValidation();
      break;
    case 5:
      lcd.clear();
      lcd.print(unfinishedRandomWord);
    //  player.play(6);
      delay(1500);
      tp[randomNumber].AnswerValidation();
      break;
    case 6:
      lcd.clear();
      lcd.print(unfinishedRandomWord);
   //   player.play(7);
      delay(1500);
      tp[randomNumber].AnswerValidation();
      break;
    case 7:
      lcd.clear();
      lcd.print(unfinishedRandomWord);
    //  player.play(8);
      delay(1500);
      tp[randomNumber].AnswerValidation();
      break;
  }
}

void WordTag::AnswerValidation() {
  if (!RFID.PICC_IsNewCardPresent() || !RFID.PICC_ReadCardSerial() ) {
    Serial.println("uau");
    return;
  }

  tagID = "";

  for (byte i = 0; i < RFID.uid.size; i++) {
    tagID.concat(String(RFID.uid.uidByte[i], HEX));
  }

  if (tagID.equalsIgnoreCase(tp[randomNumber].tag)) {
    lcd.clear();
   // player.play(13);
    lcd.print(randomWord);
    Serial.println("acertou");
    delay(4000);
    gameEnded = true;
    playerScore++;

  } else {
    tries++;
    //player.play(14);
    delay(3000);
    Serial.println("errou");

    if (tries >= 3) {
      lcd.clear();
     // player.play(15);
      lcd.print(randomWord);
      delay(4000);
      gameEnded = true;
    }
  }
};

void setup()
{
  #if (defined ESP32)
    FPSerial.begin(9600, SERIAL_8N1, 26, 27);
  #else
    FPSerial.begin(9600);
  #endif

  Serial.begin(115200);


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
  lcd.print("Fisga Letras!");
  delay(1000);
  lcd.clear();
  delay(1000);
  lcd.print("Fisga Letras!");
  delay(1000);
  lcd.clear();
  delay(1000);
  lcd.print("Fisga Letras!");
  delay(1000);
  lcd.clear();
  delay(1000);
 lcd.print("Fisga Letras!");

//  player.volume(25);
//  player.play(9);
//  delay(5000);
//  player.play(10);
//  delay(5000);
//  player.play(11);
//  delay(5000);
//  player.play(12);
//  delay(5000);

  StartGame();
}

void loop() { 
  if (gameEnded) {
    // Verifica se todos os jogadores já jogaram nesta rodada
      currentRound++; // Avança para a próxima rodada

      // Verifica se todas as rodadas foram jogadas
      if (currentRound >= maxRounds) {
        // O jogo terminou, imprime os resultados
        
          Serial.print("Jogador ");
          Serial.print(": ");
          if (playerScore >= 2) {
            Serial.println("Ganhou");
            lcd.clear();
            lcd.print("Jogador Ganhou");
            delay(2000);
          } else {
            Serial.println("Perdeu");
            lcd.clear();
            lcd.print("Jogador Perdeu");
            delay(2000);
          }
        
        // Reinicia o jogo
        currentRound = 0;
       
        playerScore = 0;
        resetFunc();
      }
    lcd.clear();
    StartGame();
    
  } else {
    tp[randomNumber].AnswerValidation();
  }

  delay(1000); 
}
