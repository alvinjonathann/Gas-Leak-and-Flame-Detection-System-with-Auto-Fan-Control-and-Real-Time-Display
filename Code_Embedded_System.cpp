#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int gasPin = A0;
const int flamePin = 2;
const int buzzerPin = 9;
const int fanPin = 5;

int gasBaseline = 0;
int threshold = 0;
unsigned long lastPrintTime = 0;

bool hitungWaktuAktif = false;
unsigned long waktuMulai = 0;
unsigned long waktuBerjalan = 0;
int nilaiGasSebelumnya = 0;
bool waktuDicatat = false;  

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.backlight();

  pinMode(flamePin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(fanPin, OUTPUT);

  tone(buzzerPin, 1000);
  delay(500);
  noTone(buzzerPin);

  lcd.setCursor(0, 0);
  lcd.print("Kalibrasi gas...");
  Serial.println("Kalibrasi gas dimulai - 10 detik");

  for (int remaining = 300; remaining > 0; remaining--) {
    lcd.setCursor(0, 1);
    lcd.print("Sisa: ");
    lcd.print(remaining);
    lcd.print(" detik   ");
    delay(1000);
  }

  lcd.setCursor(0, 1);
  lcd.print("Sampling...      ");
  Serial.println("Mengambil nilai baseline...");

  long total = 0;
  for (int i = 0; i < 100; i++) {
    int reading = analogRead(gasPin);
    total += reading;
    delay(50);
  }

  gasBaseline = total / 100;
  threshold = gasBaseline + 80;

  Serial.print("Gas baseline: ");
  Serial.println(gasBaseline);
  Serial.print("Threshold: ");
  Serial.println(threshold);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Baseline: ");
  lcd.print(gasBaseline);
  lcd.setCursor(0, 1);
  lcd.print("Monitoring Aktif");
  delay(2000);
}

void loop() {
  int gasValue = analogRead(gasPin);
  int flameValue = digitalRead(flamePin);
  unsigned long currentTime = millis();

  if (currentTime - lastPrintTime >= 1000) {
    Serial.print("Gas: ");
    Serial.print(gasValue);
    Serial.print(" | Threshold: ");
    Serial.print(threshold);
    Serial.print(" | Api: ");
    Serial.println(flameValue == LOW ? "TERDETEKSI" : "TIDAK ADA");
    lastPrintTime = currentTime;
  }

  // Start stopwatch kalau gas udah turun ke nilai <= 200
  // serial print untuk perhitungan waktu untuk dataset
  if (!hitungWaktuAktif && !waktuDicatat && gasValue <= 250 && nilaiGasSebelumnya > 250) {
    hitungWaktuAktif = true;
    waktuMulai = millis();
    Serial.println("⏳ Mulai hitung waktu dari gas <= 250");
  }

  // Stop stopwatch
  if (hitungWaktuAktif && gasValue <= threshold && digitalRead(buzzerPin) == LOW) {
    waktuBerjalan = millis() - waktuMulai;
    Serial.print("✅ Waktu dari <=250 ke threshold: ");
    Serial.print(waktuBerjalan / 1000.0);
    Serial.println(" detik");

    hitungWaktuAktif = false;
    waktuDicatat = true;  // 
    delay(500);
  }

  if (gasValue > 250 && waktuDicatat) {
    waktuDicatat = false;  
  }

  nilaiGasSebelumnya = gasValue;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gas: ");
  lcd.print(gasValue);

  bool apiTerdeteksi = (flameValue == LOW);
  bool gasBocor = (gasValue > threshold);

  if (apiTerdeteksi && gasBocor) {
    lcd.setCursor(0, 1);
    lcd.print("Fire & Gas!");
    tone(buzzerPin, 1500);
    digitalWrite(fanPin, LOW);
  } else if (apiTerdeteksi) {
    lcd.setCursor(0, 1);
    lcd.print("Fire Detected!");
    tone(buzzerPin, 1000);
    digitalWrite(fanPin, LOW);
  } else if (gasBocor) {
    lcd.setCursor(0, 1);
    lcd.print("Gas Leaked!");
    tone(buzzerPin, 2000);
    digitalWrite(fanPin, HIGH);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Area Clear");
    noTone(buzzerPin);
    digitalWrite(fanPin, LOW);
  }

  delay(100);
}
