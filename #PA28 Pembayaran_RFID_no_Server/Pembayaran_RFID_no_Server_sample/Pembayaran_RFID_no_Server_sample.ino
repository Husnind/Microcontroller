#define BLYNK_TEMPLATE_ID "TMPL6xefJzpY5"
#define BLYNK_TEMPLATE_NAME "Alat Pembayaran"
#define BLYNK_AUTH_TOKEN "H2Mzt_l9K7RdNaRKPaigDuqGQ01yKzZb"
#define BLYNK_PRINT Serial


#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Wi-Fi credentials
char ssid[] = "realme 9 Pro 5G";
char pass[] = "12345678";

#define RST_PIN     25
#define SS_PIN      5
#define TambahSaldo V1
#define JumlahBayar V2

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {12,14,27,26};
byte colPins[COLS] = {33,32,16,17};


MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

bool ceksaldo = true;
bool bayar = false;
bool isisaldo = false;
bool tambah = false;
bool kurang = false;
String batal = "";
int count = 0;
unsigned long lastmillis;

bool notif = true;
bool isiSaldo = false;
String isiSldo = "";
String input;
long saldo;
int digit;

long OLDsaldo;
int OLDdigit;

long nilaiBaru;
bool tapisisaldo = false;

String menuEntriNilai;
byte lcdEntriPos = 9;
bool entriNilai = false;


void setup() {
  Serial.begin(9600);
  SPI.begin();
  Wire.begin();
  lcd.init();
  lcd.backlight();
  // Memulai Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  mfrc522.PCD_Init();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  lcd.setCursor(0, 0);
  lcd.print("Pembayaran RFID");
  lcd.setCursor(0, 1);
  lcd.print("By: Kelompok 5");
  delay(1500);
  lcd.clear();
  delay(50);

}

void loop() {
  Blynk.run();
  cekmenu();
}


void cekmenu() {
  char keyp = keypad.getKey();
  if (ceksaldo == true) {
    if (millis() - lastmillis >= 2000) {
      if (count == 3) {
        count = 1;
      }
      else {
        count++;
      }
      lastmillis = millis();
    }

    if (count == 1) {

      lcd.setCursor(5, 0);
      lcd.print(" Scan  ");

    }
    if (count == 2) {

      lcd.setCursor(5, 0);
      lcd.print("A.Topup");

    }
    if (count == 3) {
      lcd.setCursor(5, 0);
      lcd.print("B.Bayar");
    }


    lihatsaldo();

    if (keyp == 'A') {
      lcd.clear();
      delay(50);
      ceksaldo = false;
      isisaldo = true;
      menuEntriNilai = "";
      entriNilai = true;
      lcdEntriPos = 9;
      lcd.clear();
      delay(50);
      //delay(1000);
    }

    if (keyp == 'B') {
      lcd.clear();
      delay(50);
      ceksaldo = false;
      bayar = true;
      menuEntriNilai = "";
      entriNilai = true;
      lcdEntriPos = 9;
      lcd.clear();
      delay(50);
      //delay(1000);
    }
  }
  else if (isisaldo == true) {
    lcd.setCursor(0, 0);
    lcd.print("Topup Rp.");
    //lcd.setCursor(10, 0);

    if (entriNilai) {
      if ((keyp >= '0') && (keyp <= '9') && keyp) {
        if (lcdEntriPos >= 15) {
          lcd.setCursor(0, 1);
          lcd.print("nilai lebih");
          delay(2000);
          lcd.clear();
          delay(50);
          ceksaldo = true;
          isisaldo = false;
        }
        else {
          menuEntriNilai += keyp;
          lcd.setCursor(lcdEntriPos++, 0);
          lcd.print(keyp);
        }
        nilaiBaru = menuEntriNilai.toInt();
      }

      if (keyp == '*') {
        //Serial.println(menuEntriNilai.toInt());
        tambah = true;
        isisaldo == false;
        //delay(1000);
      }

      if (keyp == '#') {
        batal = "ya";
      }
      //else {}
    }
  }
  else if (bayar == true) {
    lcd.setCursor(0, 0);
    lcd.print("Bayar Rp.");
    //lcd.setCursor(10, 0);

    if (entriNilai) {
      if ((keyp >= '0') && (keyp <= '9') && keyp) {
        if (lcdEntriPos >= 15) {
          lcd.setCursor(0, 1);
          lcd.print("nilai lebih");
          delay(2000);
          lcd.clear();
          delay(50);
          ceksaldo = true;
          isisaldo = false;
        }
        else {
          menuEntriNilai += keyp;
          lcd.setCursor(lcdEntriPos++, 0);
          lcd.print(keyp);
        }
        nilaiBaru = menuEntriNilai.toInt();
      }

      if (keyp == '*') {
        //Serial.println(menuEntriNilai.toInt());
        kurang = true;
        bayar == false;
        //delay(1000);
      }

      if (keyp == '#') {
        batal = "ya";
      }
      //else {}
    }
  }

  if (tambah == true) {
    tambah_saldo();
  }

  if (kurang == true) {
    pembayaran();
  }

  if (batal == "ya") {
    ceksaldo = true;
    isisaldo = false;
    bayar = false;
    tambah = false;
    kurang = false;
    batal = "";
    //delay(500);
    lcd.clear();
    delay(50);
    return;
  }
}

struct Transaksi {
  String tipe;    // "Top-Up" atau "Pembayaran"
  long nilai;     // Jumlah transaksi
  String waktu;   // Tanggal dan waktu transaksi
};

Transaksi riwayat[10]; // Menyimpan hingga 10 transaksi
int indexTransaksi = 0;

void lihatsaldo() {

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
  // Cek kesesuaian kartu
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Kode ini hanya dapat digunakan pada MIFARE Classic cards 1KB - 13.56MHz."));
    notif = true;
    delay(2000);
    resetReader();
    return;
  }

  // that is: sector #1, covering block #4 up to and including block #7
  byte sector         = 1;
  byte blockAddr      = 4;

  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  // Baca Saldo yang ada dari RFID Card
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Gagal Baca Kartu RFID");
    //Serial.println(mfrc522.GetStatusCodeName(status));
    resetReader();
    return;
  }
  //lcd.setCursor(0, 1);
  //lcd.print("                   ");
  OLDdigit = buffer[0];
  OLDsaldo = OLDdigit;
  OLDsaldo *= 1000;
  Serial.print("Saldo Kartu Sebelumnya : ");
  Serial.println(OLDsaldo);
  Serial.println();
  lcd.setCursor(0, 1);
  lcd.print("Saldo: RP.");
  lcd.print(OLDsaldo);
  delay(2000);
  //lcd.setCursor(0, 0);
  //lcd.print("                   ");
  lcd.setCursor(0, 1);
  lcd.print("                   ");
  //tapkartu = true;
  resetReader();
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void resetReader() {
  // Halt PICC
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

void pembayaran() {

  lcd.setCursor(0, 1);
  lcd.print("Tempelkan RFID");
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  lcd.setCursor(0, 1);
  lcd.print("                ");
  saldo = nilaiBaru;
  saldo = saldo / 1000;
  digit = saldo;
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Cek kesesuaian kartu
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Kode ini hanya dapat digunakan pada MIFARE Classic cards 1KB - 13.56MHz."));
    notif = true;
    delay(2000);
    resetReader();
    return;
  }

  // that is: sector #1, covering block #4 up to and including block #7
  byte sector         = 1;
  byte blockAddr      = 4;

  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  //Serial.println("Current data in sector:");
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  // Baca Saldo yang ada dari RFID Card
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Gagal Baca Kartu RFID");
    //Serial.println(mfrc522.GetStatusCodeName(status));
    resetReader();
    return;
  }
  OLDdigit = buffer[0];
  OLDsaldo = OLDdigit;
  OLDsaldo *= 1000;

  Serial.print("Saldo Kartu Sebelumnya : ");
  Serial.println(OLDsaldo);
  Serial.println();

  lcd.setCursor(0, 0);
  lcd.print("Saldo: Rp.");
  lcd.print(OLDsaldo);
  delay(2000);

  // Kurangi Saldo sebesar tagihan merchant
  if (OLDdigit < digit) {
    lcd.setCursor(0, 1);
    lcd.print("GAGAL Bayar");
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("Saldo Kurang");
    delay(2000);
    lcd.setCursor(0, 1);
    lcd.print("                   ");

    resetReader();
    lcd.clear();
    delay(50);
    
    ceksaldo = true;
    kurang = false;
    bayar = false;
    return;
  }

  OLDdigit -= digit;

  byte dataBlock[]    = {
    //0,      1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
    OLDdigit, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.println("GAGAL Write Saldo pada Kartu RFID");
    //Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.println();

  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Gagal Baca Kartu RFID");
    //Serial.println(mfrc522.GetStatusCodeName(status));
  }

  Serial.println();

  Serial.println("Mengurangi Saldo...");
  if (buffer[0] == dataBlock[0]) {
    saldo = buffer[0];
    saldo *= 1000;
    lcd.setCursor(0, 1);
    lcd.print("Berhasil Bayar");
    Blynk.virtualWrite(JumlahBayar, nilaiBaru);
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("Sisa: Rp.");
    lcd.print(saldo);
  }
  else {
    lcd.setCursor(0, 1);
    lcd.print("GAGAL BAYAR");
  }
  ceksaldo = true;
  kurang = false;
  bayar = false;
  //bayarmerchant = true;
  delay(3000);
  resetReader();
  lcd.clear();
  delay(50);

}

void tambah_saldo() {
  
  saldo = nilaiBaru;
  saldo = saldo / 1000;
  if (saldo > 255) {
    saldo = 0;
    isiSldo = "gagal";
    Serial.println("Saldo tidak boleh lebih dari 255000");
    lcd.setCursor(0, 1);
    lcd.print("GAGAL Input");
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("Saldo > 255000");
    delay(2000);
    lcd.clear();
    delay(50);
    menuEntriNilai = "";
    entriNilai = true;
    lcdEntriPos = 9;
    ceksaldo = false;
    tambah = false;
    isisaldo = true;
    //tapisisaldo = true;
    return;
  }
  if (saldo <= 0) {
    saldo = 0;
    lcd.setCursor(0, 1);
    lcd.print("Isi min Rp. 1000");
    delay(1000);
    lcd.clear();
    delay(50);
    menuEntriNilai = "";
    entriNilai = true;
    lcdEntriPos = 9;
    ceksaldo = false;
    tambah = false;
    isisaldo = true;
    //tapisisaldo = true;
    return;
  }

  isiSaldo = true;
  digit = saldo;
  saldo *= 1000;
  lcd.setCursor(0, 1);
  lcd.print("Tempelkan RFID");

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }


  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Cek kesesuaian kartu
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Kode ini hanya dapat digunakan pada MIFARE Classic cards 1KB - 13.56MHz."));
    notif = true;
    delay(2000);
    resetReader();
    return;
  }

  // that is: sector #1, covering block #4 up to and including block #7
  byte sector         = 1;
  byte blockAddr      = 4;

  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);


  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  if (isiSaldo) {
    // Baca Saldo yang ada dari RFID Card
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
      Serial.println("Gagal Baca Kartu RFID");
      //Serial.println(mfrc522.GetStatusCodeName(status));
      resetReader();
      return;
    }
    OLDdigit = buffer[0];
    OLDsaldo = OLDdigit;
    OLDsaldo *= 1000;

    // Tambah saldo dan Write Saldo pada RFID Card
    saldo += OLDsaldo;
    digit += OLDdigit;

    if (digit > 255) {
      saldo = 0;
      digit = 0;
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("GAGAL Topup");
      delay(1000);
      lcd.setCursor(0, 1);
      lcd.print("Saldo > 255000");
      delay(2000);
      lcd.clear();
      delay(50);
      ceksaldo = true;
      tambah = false;
      isisaldo = false;
      resetReader();
      //tapisisaldo = true;
      return;
    }

    byte dataBlock[]    = {
      //0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
      digit, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.println("GAGAL Write Saldo pada Kartu RFID");
      //Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.println();

    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
      Serial.println("Gagal Baca Kartu RFID");
      //Serial.println(mfrc522.GetStatusCodeName(status));
    }

    Serial.println();

    Serial.println("Menambahkan Saldo...");
    if (buffer[0] == dataBlock[0]) {
      //Serial.print("data digit ke 0 : ");
      //Serial.println(buffer[0]);
      Serial.print("Saldo kartu sekarang : ");
      Serial.println(saldo);
      Serial.println("_________ Berhasil isi saldo pada kartu ___________");
      Blynk.virtualWrite(TambahSaldo, nilaiBaru);
    } else {
      Serial.println("------------ GAGAL ISI SALDO --------------");
    }
  }
  isiSldo = "sukses";
  nilaiBaru = saldo;
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("Berhasil Topup");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("Saldo: Rp.");
  lcd.print(nilaiBaru);
  saldo = 0;
  digit = 0;
  ceksaldo = true;
  tambah = false;
  isisaldo = false;
  delay(3000);
  resetReader();
  lcd.clear();
  delay(50);

}
