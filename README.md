# Controle de Joystick, LEDs RGB e Display OLED

Este projeto implementa um sistema de controle utilizando um joystick analógico para manipular um display OLED SSD1306 e controlar LEDs RGB via PWM.

## Funcionalidades

- **Leitura do Joystick:**

  - Captura os valores do eixo X e Y utilizando o ADC do RP2040.
  - Converte os valores lidos para coordenadas na tela OLED e valores PWM para controle dos LEDs.

- **Controle de LEDs RGB:**

  - Modulação PWM para ajuste da intensidade dos LEDs vermelho e azul.
  - Alteração da intensidade baseada no movimento do joystick.

- **Exibição no Display OLED SSD1306:**

  - Um quadrado é desenhado na tela e sua posição varia conforme o movimento do joystick.
  - A borda do display é exibida para indicar os limites do movimento.

- **Interrupções para Botões:**

  - Botão A ativa/desativa o controle dos LEDs via PWM.
  - Pressionar o botão do joystick é ativado o Led verde e borda no display.

## Hardware Utilizado

- **Microcontrolador:** Raspberry Pi Pico (RP2040)
- **Joystick Analógico**
- **Display OLED SSD1306 (128x64) via I2C**
- **LEDs RGB conectados a GPIOs**
- **Botões de acionamento**

## Como Usar

1. Conecte os componentes conforme o esquema do código-fonte.
2. Compile e carregue o firmware na placa Raspberry Pi Pico.
3. Utilize o joystick para mover o quadrado na tela OLED.
4. Pressione o botão A para ativar/desativar os LEDs RGB controlados por PWM.
5. O sistema funciona em um loop contínuo, capturando os dados do joystick e atualizando a saída correspondente.

## Link do video:





**Discente:** Greique de araujo silva

**Curso:** Sistema embarcado (Engenharia da computação - UFRB).

