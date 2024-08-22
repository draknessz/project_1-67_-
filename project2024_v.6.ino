#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <ArtronShop_LineNotify.h>

#define LINE_TOKEN "Us6v5Zfbpw9Bjko9XrjUNevxsnjXAE6kHXqiF29palr"  // TOKEN
#define BUTTON_BORROW 2                                           // ปุ่ม ยืม
#define BUTTON_RETURN 3                                           // ปุ่ม คืน
#define BUTTON_BACK 4                                             // ปุ่ม ย้อนกลับ
#define BUTTON_CONFIRM 5                                          // ปุ่ม ยืนยัน

LiquidCrystal_I2C lcd(0x27, 20, 4);
SoftwareSerial mySerial(12, 13);  // พอร์ต RX, TX

String itemID = "";
String userID = "";
String userName = "";
String itemName = "";
String total = "";
int mode = 0;                              // 0: none, 1: borrow, 2: return
const char* ssid = "Aqua-Marine-Hoshino";  // WiFi Name
const char* password = "0927915144";       // WiFi Password

void setup() {
  Serial.begin(115200);
  while (!Serial);
  mySerial.begin(9600);

  // กำหนดพินสำหรับปุ่ม
  pinMode(BUTTON_BORROW, INPUT_PULLUP);
  pinMode(BUTTON_RETURN, INPUT_PULLUP);
  pinMode(BUTTON_BACK, INPUT_PULLUP);
  pinMode(BUTTON_CONFIRM, INPUT_PULLUP);

  // เปิดไฟแบล็คไลท์แล้วพิมพ์ข้อความ
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("WELCOME TO RMUTL");
  lcd.setCursor(5, 1);
  lcd.print("PROJECT 2024");
  lcd.setCursor(3, 2);
  lcd.print("BY ANAN SUKMEE");
  lcd.setCursor(4, 3);
  lcd.print("61623206033-0");
  delay(3000);
  lcd.clear();

  // เชื่อมต่อ WiFi
  Serial.println("\nConnecting to WiFi");
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecting.");
  WiFi.begin(ssid, password);  // เริ่มต้นเชื่อมต่อ WiFi

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(16, 0);
    lcd.print("...");
  }

  delay(2000);
  lcd.clear();
  Serial.println("\nWiFi connected.");
  lcd.setCursor(0, 0);
  lcd.print("WiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  lcd.setCursor(0, 1);
  lcd.print("IP:");
  lcd.print(WiFi.localIP());
  delay(2500);
  lcd.clear();
  LINE.begin(LINE_TOKEN);  // เริ่มต้นใช้ LINE Notify
}

void loop() {
  // ตรวจสอบข้อมูลจากซีเรียล
  if (mySerial.available()) {
    String qrData = mySerial.readStringUntil('\n');  // อ่านข้อมูล QRID จนกว่าจะเจอ '\n'
    qrData.trim();                                   // ลบช่องว่างที่ไม่จำเป็น

    if (qrData.length() > 0) {
      if (mode == 0) {
        // แสกนครั้งแรก (อ่านข้อมูลจาก URL PHP users)
        userID = qrData;
        getUserInfo(userID);
      } else if (mode == 1 || mode == 2) {
        // แสกนครั้งที่ 2 (อ่านข้อมูลจาก itemID)
        itemID = qrData;
        getItemInfo(itemID);
      }
    }
  }

  // ตรวจสอบการกดปุ่ม
  if (digitalRead(BUTTON_BORROW) == LOW) {
    if (mode == 0) {
      // กดปุ่มยืมก่อนแสกน UserID
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("PLS PUTBUTTOM BACK!");
      lcd.setCursor(0,1);
      lcd.print("To Scan UsersID");
      delay(1000);
    }
    mode = 1;  // เลือกโหมด ยืม
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mode: Borrow");
    Serial.println("Mode: Borrow");
    lcd.setCursor(0, 1);
    lcd.print("Please Scan Item");
    delay(1000);
  } else if (digitalRead(BUTTON_RETURN) == LOW) {
    if (mode == 0) {
      // กดปุ่มคืนก่อนแสกน UserID
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("PLS PUTBUTTOM BACK!");
      lcd.setCursor(0,1);
      lcd.print("To Scan UsersID");
      delay(1000);
    }
    mode = 2;  // เลือกโหมด คืน
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mode: Return");
    Serial.println("Mode: Return");
    lcd.setCursor(0, 1);
    lcd.print("Please Scan Item");
    delay(1000);
  } else if (digitalRead(BUTTON_BACK) == LOW) {
    mode = 0;  // ย้อนกลับ
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Back to scan UsersID");
    delay(1000);
  } else if (digitalRead(BUTTON_CONFIRM) == LOW) {
    if (mode == 1 || mode == 2) {
      confirmAction();
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Scan itemID first");
      delay(1000);
    }
  }
}

void getUserInfo(String userID) {
  Serial.print("userID: ");
  Serial.println(userID);
  // อ่านข้อมูลจาก URL PHP users โดยใช้ userID
  String url = "http://192.168.0.3/usersreq.php?UID=";
  url += userID;

  HTTPClient http;
  http.begin(url);

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("Response: ");
    Serial.println(payload);

    // ประมวลผล JSON ที่ได้รับ
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    userName = doc["fname"] | "No name";
    Serial.print("User Name: ");
    Serial.println(userName);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("User Name ");
    lcd.setCursor(0, 1);
    lcd.println(userName);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HTTP Error");
  }
  http.end();
}

void getItemInfo(String itemID) {
  Serial.print("itemID: ");
  Serial.print(itemID);
  // อ่านข้อมูลจาก URL PHP items โดยใช้ itemID
  String url = "http://192.168.0.3/itemreq.php?itemID=";
  url += itemID;

  HTTPClient http;
  http.begin(url);

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.print("\nHTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("Response: ");
    Serial.println(payload);

    // ประมวลผล JSON ที่ได้รับ
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    itemName = doc["item_name"] | "No item";
    total = doc["total"] | "No total";
    Serial.print("Item Name: ");
    Serial.println(itemName);
    Serial.print("total: ");
    Serial.println(total);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Item Name: ");
    lcd.print(itemName);
    lcd.setCursor(0, 1);
    lcd.print("total: ");
    lcd.print(total);
  } else {
    Serial.print("\nError code: ");
    Serial.println(httpResponseCode);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HTTP Error");
  }
  http.end();
}

void confirmAction() {
  // ส่งข้อมูลการดำเนินการไปยังฐานข้อมูล
  String url = "http://192.168.0.3/confirm.php";
  url += "?mode=" + String(mode) + "&userID=" + userID + "&itemID=" + itemID;

  HTTPClient http;
  http.begin(url);

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("Confirmed ");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Confirmed ");
    delay(500);
    

    String modeText = (mode == 1) ? "borrow" : "return";
    LINE.send("Confirmed: Mode " + modeText + " UserName: " + userName + " ItemName: " + itemName);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HTTP Error");
  }
  http.end();
    lcd.setCursor(0, 0);
    lcd.print("IF You want");
    lcd.setCursor(0, 1);
    lcd.print("borrow or return");
    lcd.setCursor(0, 2);
    lcd.print("PUT BUTTOM BACK !");
    lcd.setCursor(0, 3);
    lcd.print("THANK YOU ^_^");
}
