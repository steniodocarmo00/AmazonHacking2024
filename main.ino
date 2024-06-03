#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>

#define endereco 0x27
#define colunas 16
#define linhas 2

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))
#include <SoftwareSerial.h>
SoftwareSerial softSerial(6, 5);
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif

#define SS_PIN 10
#define RST_PIN 9

LiquidCrystal_I2C lcd(endereco, colunas, linhas);

DFRobotDFPlayerMini player;

MFRC522 RFID(SS_PIN, RST_PIN);

uint8_t SmileyFaceChar[] = {0x00, 0x00, 0x0a, 0x00, 0x1f, 0x11, 0x0e, 0x00};
const int maxPlayers = 4;
const int maxRounds = 3;
bool gameEnded = false;
bool usedWords[maxPlayers][8] = {{false}};
int currentRound = 0;
int currentPlayer = 0;
int randomNumber;
int playerScores[maxPlayers] = {0, 0, 0, 0};
int indices[8] = {0, 1, 2, 3, 4, 5, 6, 7};
String randomWord;
String randomTag;
String unfinishedRandomWord;
String tagID = "";

class WordTag
{
public:
    String tag;
    String wordStr;
    String unfinishedWord;
    int tries;
    String FindWordByTag(String tag);
    String FindUnfinishedWordByTag(String tag);
    void AnswerValidation(int playerNumber);

    WordTag(String t, String w, String u) : tag(t), wordStr(w), unfinishedWord(u), tries(0) {}
};

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

String WordTag::FindWordByTag(String tag)
{
    for (int i = 0; i < 8; i++)
    {
        if (tp[i].tag == tag)
        {
            return tp[i].wordStr;
        }
    }
    return "";
};

String WordTag::FindUnfinishedWordByTag(String tag)
{
    for (int i = 0; i < 8; i++)
    {
        if (tp[i].tag == tag)
        {
            return tp[i].unfinishedWord;
        }
    }
    return "";
};

void WordTag::AnswerValidation(int playerNumber)
{
    if (!RFID.PICC_IsNewCardPresent() || !RFID.PICC_ReadCardSerial())
    {
        return;
    }

    tagID = "";

    for (byte i = 0; i < RFID.uid.size; i++)
    {
        tagID.concat(String(RFID.uid.uidByte[i], HEX));
    }

    if (tagID.equalsIgnoreCase(tp[randomNumber].tag))
    {
        player.playMp3Folder(13);
        lcd.clear();
        lcd.print("Jogador ");
        lcd.print(playerNumber + 1);
        lcd.print(": ");
        lcd.setCursor(0, 1);
        lcd.print(randomWord);
        delay(4000);
        gameEnded = true;
        playerScores[playerNumber]++;
    }
    else
    {
        tp[randomNumber].tries++;
        player.playMp3Folder(14);
        delay(3000);

        if (tp[randomNumber].tries >= 3)
        {
            player.playMp3Folder(15);
            lcd.clear();
            lcd.print("Jogador ");
            lcd.print(playerNumber + 1);
            lcd.print(": ");
            lcd.setCursor(0, 1);
            lcd.print(randomWord);
            delay(4000);
            gameEnded = true;
        }
    }
}

void (*resetFunc)(void) = 0;

void shuffleArray(int *array, int size)
{
    for (int i = size - 1; i > 0; i--)
    {
        int j = random(i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void StartGame(int playerNumber)
{
    if (currentRound < maxRounds)
    {
        do
        {
            randomNumber = indices[random(8)];
        } while (usedWords[playerNumber][randomNumber]);

        usedWords[playerNumber][randomNumber] = true;
        randomTag = tp[randomNumber].tag;
        randomWord = tp[randomNumber].FindWordByTag(randomTag);
        unfinishedRandomWord = tp[randomNumber].FindUnfinishedWordByTag(randomTag);
        gameEnded = false;

        lcd.clear();
        lcd.print("Jogador ");
        lcd.print(playerNumber + 1);
        lcd.print(": ");
        delay(1500);
        lcd.setCursor(0, 1);
        lcd.print(unfinishedRandomWord);

        player.playMp3Folder(randomNumber + 1);
        delay(1500);
    }
}

void ShowFinalScores()
{
    player.playMp3Folder(16);
    lcd.clear();
    lcd.print("Fim de Jogo!!!");
    delay(3000);
    lcd.clear();

    lcd.print("Pontuacao Final:");
    player.playMp3Folder(18);
    delay(3000);
    player.playMp3Folder(17);
    for (int i = 0; i < maxPlayers; i++)
    {
        lcd.clear();
        lcd.print("Jogador ");
        lcd.print(i + 1);
        lcd.print(": ");
        lcd.setCursor(0, 1);
        lcd.print(playerScores[i]);
        delay(2500);
    }
    lcd.clear();
    player.playMp3Folder(19);
    lcd.setCursor(7, 0);
    lcd.write(byte(0));
    delay(4000);
    resetFunc();
}

void setup()
{
    randomSeed(analogRead(A0));
    shuffleArray(indices, 8);

#if (defined ESP32)
    FPSerial.begin(9600, SERIAL_8N1, 6, 5);
#else
    FPSerial.begin(9600);
#endif

    Serial.begin(115200);

    if (!player.begin(FPSerial, true, true))
    {
        Serial.println(F("Unable to begin:"));
        Serial.println(F("1.Please recheck the connection!"));
        Serial.println(F("2.Please insert the SD card!"));
    }
    Serial.println(F("DFPlayer Mini online."));

    lcd.init();
    lcd.backlight();
    SPI.begin();
    RFID.PCD_Init();
    lcd.createChar(0, SmileyFaceChar);
    lcd.print("Fisga Letras!");
    delay(1000);
    lcd.clear();
    delay(1000);
    lcd.print("Fisga Letras!");
    delay(1000);
    lcd.clear();
    delay(1000);
    lcd.print("Fisga Letras!");

    player.volume(30);
    player.playMp3Folder(9);
    delay(2000);
    player.playMp3Folder(10);
    delay(3000);
    player.playMp3Folder(11);
    delay(6000);
    player.playMp3Folder(12);
    delay(3000);

    StartGame(currentPlayer);
}

void loop()
{
    if (!gameEnded)
    {
        tp[randomNumber].AnswerValidation(currentPlayer);
    }
    else
    {
        currentPlayer++;
        if (currentPlayer >= maxPlayers)
        {
            currentPlayer = 0;
            currentRound++;
            if (currentRound >= maxRounds)
            {
                ShowFinalScores();
                while (true)
                {
                }
            }
        }
        StartGame(currentPlayer);
    }
}
