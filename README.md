# Sistema de Controle e Monitoramento de Motores com ESP32

<img alt="Technologies" src="https://img.shields.io/badge/MCU-ESP32-blue" /> <img alt="Technologies" src="https://img.shields.io/badge/Linguagem-C++-brightgreen" /> <img alt="Technologies" src="https://img.shields.io/badge/Protocolo-ThingSpeak-orange" /> <img alt="Technologies" src="https://img.shields.io/badge/OLED-SSD1306-red" /> <img alt="Technologies" src="https://img.shields.io/badge/PWM-ESP32-green" />

## Projeto
Este projeto visa controlar e monitorar dois motores DC utilizando um ESP32, potenciômetros para controle de velocidade e LEDs para indicação de status. O sistema também se comunica com a plataforma **ThingSpeak** para enviar os dados dos motores em tempo real e exibe informações de status em um display OLED. O projeto inclui controle de velocidade de motores via PWM, monitoramento de potenciais falhas na rede Wi-Fi, e exibição de dados em tempo real.

## Componentes Utilizados
- **ESP32** (MCU com Wi-Fi integrado)
- **Display OLED SSD1306** para visualização dos dados
- **Motores DC** (controlados via PWM)
- **Potenciômetros** (para controle da velocidade dos motores)
- **LEDs** (indicadores de status dos motores)
- **ThingSpeak** (plataforma de IoT para monitoramento remoto)

## Funcionamento do Sistema
O sistema lê os valores de dois potenciômetros, que controlam a velocidade de dois motores DC. Esses valores são mapeados e enviados para a plataforma ThingSpeak a cada 15 segundos, juntamente com o status dos motores. Além disso, o status dos motores e a conectividade com a rede Wi-Fi são exibidos em um display OLED. A plataforma ThingSpeak armazena os dados de ambos os motores para análise e visualização posterior.

## Dados Coletados
- **Motor 1 (M1):** Percentual de carga do motor 1 (valor mapeado do potenciômetro).
- **Motor 2 (M2):** Percentual de carga do motor 2 (valor mapeado do potenciômetro).

## Requisitos Atendidos
1. **Controle de Motores via PWM:** Utilizando a capacidade de PWM do ESP32 para controlar a velocidade dos motores.
2. **Monitoramento em Tempo Real:** O sistema envia os dados para ThingSpeak, permitindo o monitoramento remoto.
3. **Exibição em OLED:** O status dos motores e da conexão Wi-Fi são exibidos em tempo real no display OLED.
4. **Conexão Wi-Fi:** O ESP32 utiliza a rede Wi-Fi para se conectar à internet e enviar os dados ao ThingSpeak.
5. **Alerta de Falha de Conexão Wi-Fi:** O sistema tenta reconectar automaticamente à rede caso a conexão Wi-Fi falhe.

## Estrutura de Dados
O sistema envia as leituras de cada motor (M1 e M2) para os campos específicos de um canal no ThingSpeak:
- **Campo 1:** Status do motor 1 (percentual de carga).
- **Campo 2:** Status do motor 2 (percentual de carga).

## Visão Geral do Código

### Definição dos Pinos dos Sensores

```cpp
#define MOTOR1_PIN  12
#define MOTOR2_PIN  13
#define LED1_PIN    26
#define LED2_PIN    27
#define POT1_PIN    35
#define POT2_PIN    32
```

## Configuração do PWM para Controle dos Motores
```cpp

void setupPWM() {
  // Configurar timers e canais para PWM dos motores
  ledc_timer_config_t ledc_timer1 = { ... };
  ledc_timer_config(&ledc_timer1);
  ledc_channel_config_t ledc_channel1 = { ... };
  ledc_channel_config(&ledc_channel1);
  
  ledc_timer_config_t ledc_timer2 = { ... };
  ledc_timer_config(&ledc_timer2);
  ledc_channel_config_t ledc_channel2 = { ... };
  ledc_channel_config(&ledc_channel2);
}
```

## Leitura e Envio de Dados para ThingSpeak
```cpp

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Leitura dos potenciômetros
    int pot1Value = constrain(analogRead(POT1_PIN), 0, 4095);
    int pot2Value = constrain(analogRead(POT2_PIN), 0, 4095);

    // Mapeamento dos valores e controle dos motores via PWM
    uint32_t pwm1Value = map(pot1Value, 0, 4095, 0, (1 << PWM_RES) - 1);
    uint32_t pwm2Value = map(pot2Value, 0, 4095, 0, (1 << PWM_RES) - 1);
    setPWM(LEDC_CHANNEL_0, pwm1Value);
    setPWM(LEDC_CHANNEL_1, pwm2Value);

    // Envio de dados para ThingSpeak
    ThingSpeak.setField(1, motor1Load);
    ThingSpeak.setField(2, motor2Load);
    int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    // Atualização do display OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Status Motor:");
    display.print("M1: ");
    display.print(motor1Load);
    display.println("%");
    display.print("M2: ");
    display.print(motor2Load);
    display.println("%");
    
    if (httpCode == 200) {
      display.println("Dados enviados OK");
    } else {
      display.println("Erro no envio");
    }

    display.display();
    lastTime = millis();
  }
}
```

## Requisitos Técnicos
Plataforma ThingSpeak: Utilizada para monitoramento remoto e análise dos dados.

Controle PWM: O ESP32 utiliza PWM para controlar a velocidade dos motores de forma eficiente.

Display OLED: Para exibição local dos dados e status do sistema.

Conexão Wi-Fi: A comunicação entre o ESP32 e ThingSpeak é realizada via Wi-Fi.

## Importância do Projeto
Este projeto oferece uma solução prática para o controle remoto de motores e o monitoramento de seu desempenho. A integração com a plataforma ThingSpeak permite a coleta e análise de dados em tempo real, enquanto o display OLED fornece uma interface de monitoramento local. Isso pode ser utilizado em diversas aplicações, como sistemas automatizados de controle de motores em projetos de IoT.

## Autores
- **Gabriel Lacerda  RM: 556714**
- **Roger Cardoso  RM: 557230**
- **1ESPW**
