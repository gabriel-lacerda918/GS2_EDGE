#include <Arduino.h>
#include <WiFi.h>
#include "ThingSpeak.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "driver/ledc.h"  // Biblioteca específica para PWM do ESP32

// Definições de pinos
#define MOTOR1_PIN  12
#define MOTOR2_PIN  13
#define LED1_PIN    26
#define LED2_PIN    27
#define POT1_PIN    35
#define POT2_PIN    32

// Configurações PWM do ESP32
#define PWM_CHANNEL1 0
#define PWM_CHANNEL2 1
#define PWM_FREQ     5000
#define PWM_RES      8

// Configuração do OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configuração WiFi e ThingSpeak
const char* ssid = "Panda Network_FLILQ-2.4";
const char* password = "i5951266";
WiFiClient client;

// ThingSpeak
unsigned long myChannelNumber = 2759071;
const char * myWriteAPIKey = "2GUFF97VRLVZVEF4";
unsigned long lastTime = 0;
unsigned long timerDelay = 15000;

// Constantes de timeout e tentativas
const unsigned long WIFI_TIMEOUT = 30000; // 30 segundos
const int MAX_WIFI_ATTEMPTS = 3;

// Configuração do PWM
void setupPWM() {
  // Configurar timer 0 para MOTOR1
  ledc_timer_config_t ledc_timer1 = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .duty_resolution = (ledc_timer_bit_t)PWM_RES,
    .timer_num = LEDC_TIMER_0,
    .freq_hz = PWM_FREQ
  };
  ledc_timer_config(&ledc_timer1);

  // Configurar timer 1 para MOTOR2
  ledc_timer_config_t ledc_timer2 = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .duty_resolution = (ledc_timer_bit_t)PWM_RES,
    .timer_num = LEDC_TIMER_1,
    .freq_hz = PWM_FREQ
  };
  ledc_timer_config(&ledc_timer2);

  // Configurar canal 0 para MOTOR1
  ledc_channel_config_t ledc_channel1 = {
    .gpio_num = MOTOR1_PIN,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LEDC_CHANNEL_0,
    .timer_sel = LEDC_TIMER_0,
    .duty = 0,
    .hpoint = 0
  };
  ledc_channel_config(&ledc_channel1);

  // Configurar canal 1 para MOTOR2
  ledc_channel_config_t ledc_channel2 = {
    .gpio_num = MOTOR2_PIN,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = LEDC_CHANNEL_1,
    .timer_sel = LEDC_TIMER_1,
    .duty = 0,
    .hpoint = 0
  };
  ledc_channel_config(&ledc_channel2);
}

void setup() {
  Serial.begin(115200);

  // Inicializar OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Erro ao iniciar o display OLED");
    while (1);
  }
  
  // Configuração inicial do display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  // Configurar LEDs
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  // Configurar PWM para os motores
  setupPWM();

  // Configurar WiFi
  WiFi.mode(WIFI_STA);
  if (connectWiFi()) {
    ThingSpeak.begin(client);
    showStatus("WiFi Conectado", true);
  } else {
    showStatus("Falha WiFi", false);
  }
}

// Função para definir o duty cycle do PWM
void setPWM(uint8_t channel, uint32_t value) {
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)channel, value);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, (ledc_channel_t)channel);
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Verificar conexão WiFi
    if (WiFi.status() != WL_CONNECTED) {
      showStatus("Reconectando...", false);
      if (!connectWiFi()) {
        showStatus("Falha WiFi", false);
        lastTime = millis();
        return;
      }
    }

    // Leitura e validação dos potenciômetros
    int pot1Value = constrain(analogRead(POT1_PIN), 0, 4095);
    int pot2Value = constrain(analogRead(POT2_PIN), 0, 4095);

    // Mapeamento das leituras
    int motor1Load = map(pot1Value, 0, 4095, 0, 100);
    int motor2Load = map(pot2Value, 0, 4095, 0, 100);

    // Controle dos motores via PWM
    uint32_t pwm1Value = map(pot1Value, 0, 4095, 0, (1 << PWM_RES) - 1);
    uint32_t pwm2Value = map(pot2Value, 0, 4095, 0, (1 << PWM_RES) - 1);
    setPWM(LEDC_CHANNEL_0, pwm1Value);
    setPWM(LEDC_CHANNEL_1, pwm2Value);

    // Controle dos LEDs
    digitalWrite(LED1_PIN, motor1Load > 50 ? HIGH : LOW);
    digitalWrite(LED2_PIN, motor2Load > 50 ? HIGH : LOW);

    // Preparar dados para ThingSpeak
    ThingSpeak.setField(1, motor1Load);
    ThingSpeak.setField(2, motor2Load);

    // Enviar dados ao ThingSpeak
    int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    // Atualizar display
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Status Motor:");
    display.print("M1: ");
    display.print(motor1Load);
    display.println("%");
    display.print("M2: ");
    display.print(motor2Load);
    display.println("%");
    
    // Mostrar status do envio
    display.println();
    if (httpCode == 200) {
      display.println("Dados enviados OK");
    } else {
      display.println("Erro no envio");
      Serial.println("Erro " + String(httpCode));
    }
    
    display.display();
    lastTime = millis();
  }
}

bool connectWiFi() {
  unsigned long startAttemptTime = millis();
  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && 
         millis() - startAttemptTime < WIFI_TIMEOUT && 
         attempts < MAX_WIFI_ATTEMPTS) {
    
    attempts++;
    showStatus("Tentativa " + String(attempts), false);
    
    WiFi.begin(ssid, password);
    delay(5000);
  }

  return WiFi.status() == WL_CONNECTED;
}

void showStatus(String message, bool success) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Status:");
  display.println(message);
  if (success) {
    display.println("IP: " + WiFi.localIP().toString());
  }
  display.display();
}