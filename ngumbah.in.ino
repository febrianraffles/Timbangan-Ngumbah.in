#include <WiFiManager.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HX711.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DOUT_PIN 14
#define SCK_PIN 13
#define CONFIRM_SEND_BUTTON 33
#define TARE_BUTTON 32
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

HX711 scale;
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
bool confirmSendButtonPressed = false;
bool tareButtonPressed = false;
const char* host = "192.168.1.13";  // Ganti dengan alamat IP server Anda


HTTPClient http;

void setup() {
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Ngumbah.in");
  lcd.setCursor(0, 1);
  lcd.print("Starting");
  lcd.noBacklight();


  // Inisialisasi WiFi Manager
  WiFiManager wm;
  if (!digitalRead(0)) {
    wm.resetSettings();
  }

  // Coba koneksikan ke WiFi
  bool res = wm.autoConnect("Ngumbah.in", "12345678");

  if (!res) {
    lcd.print("...Fail");
    lcd.noBacklight();  // Matikan backlight jika tidak terkoneksi dengan WiFi
  } else {
    lcd.print("...Done");
    lcd.backlight();  // Hidupkan backlight jika terkoneksi dengan WiFi
  }

  scale.begin(DOUT_PIN, SCK_PIN);
  scale.set_scale();
  scale.tare();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ngumbah.in");
  lcd.setCursor(0, 1);
  lcd.print("Berat: ");
  lcd.print(0.00, 2);

  pinMode(CONFIRM_SEND_BUTTON, INPUT_PULLUP);
  pinMode(TARE_BUTTON, INPUT_PULLUP);
  pinMode(0, INPUT);
}

void loop() {
  if (digitalRead(CONFIRM_SEND_BUTTON) == LOW && !confirmSendButtonPressed) {
    confirmSendButtonPressed = true;
    float berat_kg = scale.get_units(10);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Simpan & Kirim...");
    lcd.setCursor(0, 1);
    lcd.print("Berat: ");
    lcd.print(berat_kg, 2);
    lcd.print(" kg");

    sendDataToServer(berat_kg);

    scale.tare();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Berhasil!");
    lcd.setCursor(0, 1);
    lcd.print("Ngumbah.in");

    delay(3000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ngumbah.in");
    lcd.setCursor(0, 1);
    lcd.print("Berat: ");
    lcd.print(0.00, 2);

    confirmSendButtonPressed = false;
  }

  if (digitalRead(TARE_BUTTON) == LOW) {
    if (!tareButtonPressed) {
      tareButtonPressed = true;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Taring...");
      lcd.setCursor(0, 1);
      lcd.print("Please wait.");
      scale.tare();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ngumbah.in");
      lcd.setCursor(0, 1);
      lcd.print("Berat: 0.00 kg");
    }
  } else {
    tareButtonPressed = false;
  }

  float berat_kg = scale.get_units(10);
  lcd.setCursor(7, 1);
  lcd.print(berat_kg, 2);
  lcd.print(" kg");
}

void sendDataToServer(float berat) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = "http://" + String(host) + "/laundry-main/pegawai/receive_data.php?berat=" + String(berat);

    http.begin(url);

    int httpCode = http.GET();
    if (httpCode > 0) {
      String response = http.getString();
      Serial.println(response);
    } else {
      Serial.println("Error on HTTP request");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}
