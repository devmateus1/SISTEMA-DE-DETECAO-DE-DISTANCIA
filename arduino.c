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
#define DIST_MAX 30      // Tamanho da prateleira (cm)
#define TAM_CAIXA 14     // Tamanho da caixa pequena (cm)
#define MAX_CAIXAS 2     // Número máximo que realmente cabe

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

// Leitura com média de 3 amostras
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

void beepMudanca() {
  tone(BUZZER, 2500);
  delay(250);
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

  // --- NOVA LOGICA ---
  // 0 cm → 2 caixas
  // 30 cm → 0 caixas
  int caixas = map(distancia, 0, DIST_MAX, MAX_CAIXAS, 0);

  if (caixas < 0) caixas = 0;
  if (caixas > MAX_CAIXAS) caixas = MAX_CAIXAS;

  // Situação do estoque
  String status;
  if (caixas == 0) status = "Vazio";
  else if (caixas == 1) status = "Regular";
  else status = "Bom";

  // Detecta mudança
  if (caixas != caixas_anterior) {
    if (caixas_anterior != -1) {
      beepMudanca();
    }
    caixas_anterior = caixas;
  }

  // LEDs
  if (caixas == 0) {
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_VERDE, LOW);
  }
  else if (caixas == 2) {
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, HIGH);
  }
  else {
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, LOW);
  }

  // LCD
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
