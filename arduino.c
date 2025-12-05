#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- LCD I2C ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- SENSOR HC-SR04 ---
#define TRIG_PIN 8
#define ECHO_PIN 9

// --- LEDS E BUZZER ---
#define LED_VERMELHO 4
#define LED_VERDE    3
#define BUZZER       6

// --- CONFIGURAÇÕES ---
#define DIST_MAX 30
#define MAX_CAIXAS 2

int caixas_anterior = -1;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("Estoque Linear");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(2000);
  lcd.clear();
}

// Leitura com média
long readDistance() {
  long total = 0;
  int valid = 0;

  for (int i = 0; i < 3; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    if (duration > 0) {
      total += duration;
      valid++;
    }
    delay(50);
  }

  return (valid > 0) ? total / valid : 0;
}

// --- BEEP ALTÍSSIMO PARA MUDANÇA ---
void beepMudanca() {
  tone(BUZZER, 5000);   // 5 kHz = muito audível
  delay(350);
  noTone(BUZZER);
}

void loop() {
  long duration = readDistance();

  if (duration == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Dist: --- cm   ");
    lcd.setCursor(0, 1);
    lcd.print("Erro: sem eco  ");
    return;
  }

  float distancia = duration * 0.0343 / 2.0;

  int caixas = map(distancia, 0, DIST_MAX, MAX_CAIXAS, 0);
  if (caixas < 0) caixas = 0;
  if (caixas > MAX_CAIXAS) caixas = MAX_CAIXAS;

  String status;
  if (caixas == 0) status = "Ruim";
  else if (caixas == 1) status = "Regular";
  else status = "Bom";

  // BEEP ALTO QUANDO A QUANTIDADE MUDA
  if (caixas != caixas_anterior) {
    if (caixas_anterior != -1) {
      beepMudanca();
    }
    caixas_anterior = caixas;
  }

  // --- LEDS E BUZZER ---
  if (caixas == 0) {   // Estoque ruim
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_VERDE, LOW);
    tone(BUZZER, 2500);   // Alarme constante

  } else if (caixas == 2) {  // Bom
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, HIGH);
    noTone(BUZZER);

  } else {  // Regular
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_VERDE, HIGH);
    noTone(BUZZER);
  }

  // --- LCD ---
  lcd.setCursor(0, 0);
  lcd.print("Dist: ");
  lcd.print((int)distancia);
  lcd.print(" cm   ");

  lcd.setCursor(0, 1);
  lcd.print("Cx:");
  lcd.print(caixas);
  lcd.print(" ");
  lcd.print(status);
  lcd.print("   ");

  delay(600);
}