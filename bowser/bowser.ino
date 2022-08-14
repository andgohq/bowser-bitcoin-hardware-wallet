#include <M5Core2.h>
#include <EEPROM.h>
#include <Electrum.h>
#include <AXP192.h>

#include "gameimg.c"
#include "walletimg.c"
#include "decoy.h"
#include "Bitcoin.h"
#include "Hash.h"
#include "SPIFFS.h"
#include "PSBT.h"

bool buttonA = false;
bool buttonB = false;
bool buttonC = false;
bool confirm = false;
bool loopMenu = true;
bool sdAvailable = false;

unsigned long timy;
int menuItem = 1;
String passKey;
String morseLetter;
String passHide;
String seedGenerateStr;
String savedSeed;
String seedGenerateArr[24];
String sdCommand;
String hashed;
String savedPinHash;
String privateKey;
String pubKey;

AXP192 power;


char ref[2][36][7] = {
    {"10", "0111", "0101", "011", "1", "1101", "001", "1111", "11", "1000", "010", "1011", "00", "01", "000", "1001", "0010", "101", "111", "0", "110", "1110", "100", "0110", "0100", "0011", "10000", "11000", "11100", "11110", "11111", "01111", "00111", "00011", "00001", "00000"},
    {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0"}};

//========================================================================
void setup(void)
{
  M5.begin();
  M5.Lcd.setBrightness(50);
  M5.Lcd.fillScreen(BLACK);

  // ADNGO: Check SD card
  sdChecker();
  if (!sdAvailable){
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.println("");
    M5.Lcd.setCursor(0, 90);
    M5.Lcd.println("Insert a valid SD Card");
    writeFile(SD, "/bowser.txt", "");
    delay(500);
    esp_restart();

  }

  // ANDGO: Run Tetris for decoy :P
  // decoySetup();

  M5.Lcd.fillScreen(BLACK);

  // ANDGO: Read file key.txt from SPIFFS (SPI Flash File System) 
  if (!SPIFFS.begin(true))
  {
    return;
  }
  File otherFile = SPIFFS.open("/key.txt");
  savedSeed = otherFile.readStringUntil('\n');
  otherFile.close();

  if (savedSeed.length() < 30)
    {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.println("No wallet found on device");
    delay(1000);
    wipeDevice();
    pinMaker();
    writeFile(SD, "/bowser.txt", "");
    loopToReset();
  }

  // ANDGO: Check if SD card has a coomand
  if (sdCommand == "HARD RESET")
  {
    wipeDevice();
    pinMaker();
    writeFile(SD, "/bowser.txt", "");
    loopToReset();
  }

  if (sdCommand.substring(0, 7) == "RESTORE")
  {
    wipeSpiffs();
    restoreFromSeed(sdCommand.substring(8, sdCommand.length()));
    pinMaker();
    writeFile(SD, "/bowser.txt", "");
    loopToReset();
  }

  enterPin(false);
  M5.Lcd.drawBitmap(0, 0, 320, 240, (uint8_t *)WalletImg_map);
  delay(3000);
}

//========================================================================
void loop()
{
  M5.update();
  loopMenu = true;
  while (loopMenu == true)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(38, 30);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.println("BOWSER WALLET");
    M5.Lcd.println("");
    M5.Lcd.setTextSize(2);
    if (menuItem == 1)
    {
      M5.Lcd.setTextColor(BLUE);
      M5.Lcd.println("Display Address");
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.println("Sign Transaction");
      M5.Lcd.println("Export ZPUB");
      M5.Lcd.println("Show Seed");
    }
    else if (menuItem == 2)
    {
      M5.Lcd.println("Display Address");
      M5.Lcd.setTextColor(BLUE);
      M5.Lcd.println("Sign Transaction");
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.println("Export ZPUB");
      M5.Lcd.println("Show Seed");
    }
    else if (menuItem == 3)
    {
      M5.Lcd.println("Display Address");
      M5.Lcd.println("Sign Transaction");
      M5.Lcd.setTextColor(BLUE);
      M5.Lcd.println("Export ZPUB");
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.println("Show Seed");
    }
    else if (menuItem == 4)
    {
      M5.Lcd.println("Display Address");
      M5.Lcd.println("Sign Transaction");
      M5.Lcd.println("Export ZPUB");
      M5.Lcd.setTextColor(BLUE);
      M5.Lcd.println("Show Seed");
      M5.Lcd.setTextColor(GREEN);
    }

    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.println("    up      down    select");

    while (buttonA == false)
    {
      if (M5.BtnA.wasReleased())
      {
        menuItem--;
        buttonA = true;
        shortVibration();
      }
      else if (M5.BtnB.wasReleased())
      {
        menuItem++;
        buttonA = true;
        shortVibration();
      }
      else if (M5.BtnC.wasReleased())
      {
        loopMenu = false;
        buttonA = true;
        longVibration();
      }
      M5.update();
    }
    buttonA = false;
    if (menuItem < 1)
    {
      menuItem = 4;
    }
    else if (menuItem > 4)
    {
      menuItem = 1;
    }
  }

  if (menuItem == 1) // ANDGO: Display Address
  {
    displayAddress();
  }
  else if (menuItem == 2) // ANDGO: Sign Transaction
  {
    signPSBT();
  }
  else if (menuItem == 3) // ANDGO: EPort ZPUB
  {
    exportMaster();
  }
  else if (menuItem == 4) // ANDGO: Show Seed
  {
    showSeed();
  }
}

//========================================================================

void displayAddress()
{
  sdChecker();
  HDPublicKey hd(pubKey);

  File otherFile = SPIFFS.open("/num.txt");
  String pubNumm = otherFile.readStringUntil('\n');
  otherFile.close();
  int pubNum = pubNumm.toInt() + 1;
  File file = SPIFFS.open("/num.txt", FILE_WRITE);
  file.print(pubNum);
  file.close();

  String path = String("m/0/") + pubNum;
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.println("      ADDRESS");
  String freshPub = hd.derive(path).address();
  M5.Lcd.qrcode(freshPub, 5, 46, 160, 3);
  M5.Lcd.setTextSize(2);
  int i = 0;
  while (i < freshPub.length() + 1)
  {
    M5.Lcd.println("              " + freshPub.substring(i, i + 12));
    i = i + 12;
  }
  sdChecker();
  if (sdAvailable)
  {
    writeFile(SD, "/bowser.txt", freshPub.c_str());
    M5.Lcd.println("");
    M5.Lcd.println("");
    M5.Lcd.println("");
    M5.Lcd.println("                saved to");
    M5.Lcd.println("                sd card");
  }
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.println("                   go back");

  while (buttonC == false)
  {
    if (M5.BtnC.wasReleased())
    {
      buttonC = true;
      shortVibration();
    }
    M5.update();
  }
  buttonC = false;
}

//========================================================================

void signPSBT()
{
  sdChecker();
  if (sdCommand.substring(0, 4) == "SIGN")
  {
    String eltx = sdCommand.substring(5, sdCommand.length() + 1);
    PSBT tx;
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.println("");
    M5.Lcd.setCursor(0, 90);
    M5.Lcd.println("  Bwahahahaha!");
    M5.Lcd.println("");
    M5.Lcd.println("  Transaction");
    M5.Lcd.println("  found");
    delay(3000);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextSize(2);

    tx.parseBase64(eltx);
    // int len_parsed = tx.parse(eltx);
    // if (len_parsed == 0)
    // {
    //   M5.Lcd.println("Can't parse tx");
    //   return;
    // }
    for (int i = 0; i < tx.tx.outputsNumber; i++)
    {
      M5.Lcd.print(tx.tx.txOuts[i].address());
      M5.Lcd.print("\n-> ");
      // Serial can't print uint64_t, so convert to int
      M5.Lcd.print(int(tx.tx.txOuts[i].amount));
      M5.Lcd.println(" sat\n");
    }
    M5.Lcd.print("Fee: ");
    M5.Lcd.print(int(tx.fee()));
    M5.Lcd.println(" sat");
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.println(" sign               cancel");
    while (buttonA == false && buttonC == false)
    {
      if (M5.BtnA.wasReleased())
      {
        buttonA = true;
        longVibration();
      }
      if (M5.BtnC.wasReleased())
      {
        buttonC = true;
        shortVibration();
      }
      M5.update();
    }
    if (buttonC == true)
    {
      buttonC = false;
      return;
    }
    buttonA = false;
    HDPrivateKey hd(savedSeed, passKey);
    HDPrivateKey account = hd.derive("m/84'/0'/0'/");
    tx.sign(account);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextSize(2);
    String signedTx = tx;
    int str_len = signedTx.length() + 1;
    char char_array[str_len];
    signedTx.toCharArray(char_array, str_len);
    writeFile(SD, "/bowser.txt", char_array);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("    Saved to SD");
    M5.Lcd.println("");
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.println("                   go back");

    while (buttonC == false)
    {
      if (M5.BtnC.wasReleased())
      {
        buttonC = true;
        shortVibration();
      }
      M5.update();
    }
    buttonC = false;
    sdCommand = "";
  }
  else if (sdAvailable)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.println("    No SD Available");
    delay(3000);
  }
  else
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.println("    Not found on SD");
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.println("                   go back");
    while (buttonC == false)
    {
      if (M5.BtnC.wasReleased())
      {
        buttonC = true;
        shortVibration();
      }
      M5.update();
    }
    buttonC = false;
  }
}

//========================================================================

void exportMaster()
{
  sdChecker();
  if (sdAvailable)
  {
    int str_len = pubKey.length() + 1;
    char char_array[str_len];
    pubKey.toCharArray(char_array, str_len);
    writeFile(SD, "/bowser.txt", char_array);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextSize(3);
    M5.Lcd.println("   EXPORT ZPUB");
    M5.Lcd.qrcode(pubKey, 5, 46, 160);
    M5.Lcd.setTextSize(2);
    int i = 0;
    while (i < pubKey.length() + 1)
    {
      M5.Lcd.println("              " + pubKey.substring(i, i + 12));
      i = i + 12;
    }
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.println("                   go back");
    while (buttonC == false)
    {
      if (M5.BtnC.wasReleased())
      {
        buttonC = true;
        shortVibration();
      }
      M5.update();
    }
    buttonC = false;
    sdCommand = "";
  }
  else
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.println("    No SD Available");
    delay(3000);
  }
}

//========================================================================

void showSeed()
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.println("    SHOW SEED");
  M5.Lcd.println("");
  M5.Lcd.setTextColor(BLUE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println(savedSeed);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.println("                   go back");
  while (buttonC == false)
  {
    if (M5.BtnC.wasReleased())
    {
      buttonC = true;
      shortVibration();
    }
    M5.update();
  }
  buttonC = false;
}

//========================================================================

void wipeDevice()
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.println("RESET/WIPE DEVICE");
  M5.Lcd.setCursor(0, 90);
  M5.Lcd.setTextColor(RED);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("Device will be reset,");
  M5.Lcd.println("are you sure?");
  M5.Lcd.println("");
  M5.Lcd.println("Insert SD card");
  M5.Lcd.println("   and press continue");
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.println("continue            cancel");

  while (buttonA == false && buttonC == false)
  {
    if (M5.BtnA.wasReleased())
    {
      buttonA = true;
      longVibration();
    }
    if (M5.BtnC.wasReleased())
    {
      buttonC = true;
      shortVibration();
    }
    M5.update();
  }
  if (buttonA == true)
  {
    wipeSpiffs();
    seedMaker();
  } else {
    loopToReset();
  }

  buttonA = false;
  buttonC = false;
}

//========================================================================

void seedChecker()
{
  File otherFile = SPIFFS.open("/key.txt");
  savedSeed = otherFile.readStringUntil('\n');
  otherFile.close();
  int seedCount = 0;

  for (int x = 0; x < 24; x++)
  {
    for (int z = 0; z < 2048; z++)
    {
      if (getValue(savedSeed, ' ', x) == seedWords[z])
      {
        seedCount = seedCount + 1;
      }
    }
  }

  if (int(seedCount) != 24)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 90);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("   Error: Reset device");
    M5.Lcd.println("   or restore from seed");
    M5.Lcd.println("   (See documentation)");
    while(1);
  }
  else
  {
    return;
  }
}

//========================================================================
void seedMaker()
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("   Write seed words");
  M5.Lcd.println("   somewhere safe!");
  waitOK();
  buttonA = false;

  byte arr[32];
  for (int i = 0; i < sizeof(arr); i++)
  { 
    arr[i] = esp_random() % 256;
  }
  seedGenerateStr = mnemonicFromEntropy(arr, sizeof(arr));

  for (int z = 0; z < 24; z++)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 70);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("   Word " + String(z + 1));
    M5.Lcd.println("");
    M5.Lcd.setTextSize(5);
    M5.Lcd.setTextColor(BLUE);
    M5.Lcd.println("  " + getValue(seedGenerateStr, ' ', z));
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.println("  next                  ");

    while (buttonA == false)
    {
      if (M5.BtnA.wasReleased())
      {
        buttonA = true;
        shortVibration();
      }
      M5.update();
    }
    buttonA = false;
  }
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("   Time to check");
  M5.Lcd.println("   the words!");
  waitOK();
  for (int z = 0; z < 24; z++)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 70);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("   Word " + String(z + 1));
    M5.Lcd.println("");
    M5.Lcd.setTextSize(5);
    M5.Lcd.setTextColor(BLUE);
    M5.Lcd.println("  " + getValue(seedGenerateStr, ' ', z));
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.println("  next                  ");


    while (buttonA == false)
    {
      if (M5.BtnA.wasReleased())
      {
        buttonA = true;
        shortVibration();
      }
      M5.update();
    }
    buttonA = false;
  }
  // M5.Lcd.fillScreen(BLACK);
  // M5.Lcd.setCursor(0, 100);
  // M5.Lcd.println("   Words will also");
  // M5.Lcd.println("   be saved to SD");

  File file = SPIFFS.open("/key.txt", FILE_WRITE);
  file.print(seedGenerateStr.substring(0, seedGenerateStr.length()) + "\n");
  file.close();

  // ANDGO: The generated seed is not saved SD card for safety reason.
  // String seedGen = "Keep you seed phrase safe but dont lose them! \n" + seedGenerateStr + "\n To learn more about seed phrases visit https://en.bitcoin.it/wiki/Seed_phrase";
  // int str_len = seedGen.length() + 1;
  // char char_array[str_len];
  // seedGen.toCharArray(char_array, str_len);

  File otherFile = SPIFFS.open("/key.txt");
  savedSeed = otherFile.readStringUntil('\n');
  otherFile.close();

  // writeFile(SD, "/bowser.txt", char_array);
}

//========================================================================

void pinMaker()
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 90);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.println("   Enter pin using");
  M5.Lcd.println("   use morse code,");
  M5.Lcd.println("   3 letters at least");
  waitOK();
  enterPin(true);
}

//========================================================================

void enterPin(bool set)
{
  passKey = "";
  passHide = "";
  morseLetter = "";
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setCursor(0, 10);
  M5.Lcd.setTextSize(3);
  M5.Lcd.print(" Morse Code pin");
  M5.Lcd.setCursor(0, 180);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println(" pause between values");
  M5.Lcd.println("");
  M5.Lcd.println("    o      ----    submit");

  confirm = false;
  while (confirm == false)
  {
    if (M5.BtnA.wasReleased())
    {
      buttonA = true;
      shortVibration();
      morseLetter = morseLetter + "1";
      timy = millis();
    }
    if (M5.BtnB.wasReleased())
    {
      buttonB = true;
      longVibration();
      morseLetter = morseLetter + "0";
      timy = millis();
    }
    if (M5.BtnC.wasReleased())
    {
      longVibration();
      if (set == true)
      {
        uint8_t newPassKeyResult[32];
        sha256(passKey, newPassKeyResult);
        hashed = toHex(newPassKeyResult, 32);

        File file = SPIFFS.open("/pass.txt", FILE_WRITE);
        file.print(hashed + "\n");
        file.close();
      }

      File otherFile = SPIFFS.open("/pass.txt");
      savedPinHash = otherFile.readStringUntil('\n');
      otherFile.close();

      uint8_t passKeyResult[32];
      sha256(passKey, passKeyResult);
      hashed = toHex(passKeyResult, 32);

      if (savedPinHash == hashed || set == true)
      {
        getKeys(savedSeed, passKey);
        passHide = "";
        confirm = true;
        return;
      }
      else if (savedPinHash != hashed && set == false)
      {
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 110);
        M5.Lcd.setTextSize(2);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.print("   Reset and try again");
        passKey = "";
        passHide = "";
        delay(3000);
      }
    }
    M5.update();
    if ((millis() - timy) > 2000)
    {
      if (buttonA == true || buttonB == true)
      {
        for (int z = 0; z < 36; z++)
        {
          if (morseLetter == ref[0][z])
          {
            passKey += ref[1][z];
            passHide += "* ";
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0, 10);
            M5.Lcd.setTextSize(3);
            M5.Lcd.print(" Morse Code pin");
            M5.Lcd.setCursor(0, 90);
            M5.Lcd.setTextColor(GREEN);
            if (set == true)
            {
              M5.Lcd.print("   " + passKey);
            }
            else
            {
              M5.Lcd.print("   " + passHide);
            }
            M5.Lcd.setCursor(0, 180);
            M5.Lcd.setTextSize(2);
            M5.Lcd.println(" pause between values");
            M5.Lcd.println("");
            M5.Lcd.println("    o      ----    submit");
          }
        }
        buttonA = false;
        buttonB = false;
        morseLetter = "";
      }
    }
  }
  confirm = false;
}

//========================================================================

void restoreFromSeed(String theSeed)
{

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.println("RESTORE FROM SEED");
  M5.Lcd.setCursor(0, 85);
  M5.Lcd.setTextColor(RED);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("Device will be wiped");
  M5.Lcd.println("then restored from seed");
  M5.Lcd.println("are you sure?");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.println("continue            cancel");

  while (buttonA == false && buttonC == false)
  {
    if (M5.BtnA.wasReleased())
    {
      buttonA = true;
      longVibration();
    }
    if (M5.BtnC.wasReleased())
    {
      buttonC = true;
      shortVibration();
    }
    M5.update();
  }
  if (buttonA == true)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("     Saving seed...");
    delay(2000);
    File file = SPIFFS.open("/key.txt", FILE_WRITE);
    file.print(theSeed + "\n");
    file.close();
    File otherFile = SPIFFS.open("/key.txt");
    savedSeed = otherFile.readStringUntil('\n');
    otherFile.close();
  } else {
    loopToReset();
  }

  buttonA = false;
  buttonC = false;
}

//========================================================================

void wipeSpiffs()
{
  File fileKey = SPIFFS.open("/key.txt", FILE_WRITE);
  fileKey.print("\n");
  fileKey.close();
  File fileNum = SPIFFS.open("/num.txt", FILE_WRITE);
  fileNum.print("\n");
  fileNum.close();
  File filePass = SPIFFS.open("/pass.txt", FILE_WRITE);
  filePass.print("\n");
  filePass.close();
}

//========================================================================

void sdChecker()
{
  File file = SD.open("/bowser.txt");
  if (!file)
  {
    sdAvailable = false;
    return;
  }
  sdAvailable = true;
  while (file.available())
  {
    sdCommand = file.readStringUntil('\n');
  }
}

//========================================================================

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//========================================================================

void getKeys(String mnemonic, String password)
{

  HDPrivateKey hd(mnemonic, password);

  if (!hd)
  { // check if it is valid
    return;
  }

  HDPrivateKey account = hd.derive("m/84'/0'/0'/");

  privateKey = account;

  pubKey = account.xpub();
}

//========================================================================



//========================================================================

void writeFile(fs::FS &fs, const char *path, const char *message)
{

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    return;
  }
  file.print(message);
}

//========================================================================

void writeIntIntoEEPROM(int addresss, int number)
{
  EEPROM.write(addresss, number);
}
int readIntFromEEPROM(int addresss)
{
  return EEPROM.read(addresss);
}


//========================================================================

void loopToReset()
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.println("");
  M5.Lcd.setCursor(0, 90);
  M5.Lcd.println("  Push reset button");
  M5.Lcd.println("  to restart");
  while(1);
}

void waitOK()
{
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.println("                      OK  ");

  while (buttonC == false)
  {
    if (M5.BtnC.wasReleased())
    {
      buttonC = true;
      longVibration();
    }
    M5.update();
  }
  buttonC = false;

  delay(100);
}

void shortVibration() {
 power.SetLDOEnable(3, true);
 delay(70);                   
 power.SetLDOEnable(3, false); 
}

void longVibration() {
 power.SetLDOEnable(3, true);
 delay(400);                   
 power.SetLDOEnable(3, false); 
}
