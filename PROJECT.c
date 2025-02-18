#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "lib/ssd1306.h"

#define GPIO_BOTAO_A    5       // Pino para o botão A
#define GPIO_LED_R      13      // Pino para o LED vermelho
#define GPIO_LED_G      11      // Pino para o LED verde
#define GPIO_LED_B      12      // Pino para o LED azul
#define GPIO_BTN_JOY    22      // Pino para o botão do joystick
#define GPIO_VRX_JOY    26      // Pino para o eixo X do joystick (analogico)
#define GPIO_VRY_JOY    27      // Pino para o eixo Y do joystick (analogico)

#define I2C_PORT        i2c1    // I2C1 será usado para comunicação com o display
#define GPIO_I2C_SDA    14     // Pino SDA para comunicação I2C
#define GPIO_I2C_SLC    15     // Pino SCL para comunicação I2C
#define ADDRESS         0x3C   // Endereço do display SSD1306

#define PWM_FREQUENCY   50     // Frequência de 50Hz para o servo motor
#define CLOCK_BASE      125000000 // Clock base de 125 MHz
#define PWM_DIVISER     125.0  // Divisor de clock para PWM

static volatile uint32_t last_time = 0;
ssd1306_t ssd;                // Estrutura para o display SSD1306
static volatile int  adc_valor_x; // Valor do eixo X do joystick
static volatile int  adc_valor_y; // Valor do eixo Y do joystick
uint16_t wrapValue = CLOCK_BASE / (PWM_FREQUENCY * PWM_DIVISER); // Valor de wrap para PWM

static void gpio_irq_handler(uint gpio, uint32_t events); // Função para interrupções de GPIO
bool pwm_leds_ativos = false; // Variável para controlar se o PWM dos LEDs RGB está ativo
bool bordaAtiva = false; // Estado da borda do display


// Função para controlar o estado dos LEDs (acesos ou apagados)
void controlaLed(uint gpio, bool operacao) {
    gpio_put(gpio, operacao);  // Define o estado do LED
}

// Função de interrupção para os botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time()); // Captura o tempo atual

    if (current_time - last_time > 200000) { // Debounce do botão
        last_time = current_time;

        if (gpio == GPIO_BOTAO_A) { // Quando o botão A é pressionado
            pwm_leds_ativos = !pwm_leds_ativos; // Alterna o estado do PWM dos LEDs RGB
        } else if (gpio == GPIO_BTN_JOY) { // Quando o botão do joystick é pressionado
            controlaLed(GPIO_LED_G, !gpio_get(GPIO_LED_G)); // Alterna o LED verde
            ssd1306_rect(&ssd, 3, 3, 119, 58, true, false);// Desenha a borda principal 
            ssd1306_send_data(&ssd); // Envia os dados para o display
        }
    }
}

// Inicializa os LEDs
void init_leds() {
    gpio_init(GPIO_LED_R);
    gpio_set_dir(GPIO_LED_R, GPIO_OUT);

    gpio_init(GPIO_LED_G);
    gpio_set_dir(GPIO_LED_G, GPIO_OUT);

    gpio_init(GPIO_LED_B);
    gpio_set_dir(GPIO_LED_B, GPIO_OUT);
}

// Inicializa os botões
void init_botoes() {
    gpio_init(GPIO_BOTAO_A);
    gpio_set_dir(GPIO_BOTAO_A, GPIO_IN);
    gpio_pull_up(GPIO_BOTAO_A); // Habilita o pull-up para o botão

    gpio_init(GPIO_BTN_JOY);
    gpio_set_dir(GPIO_BTN_JOY, GPIO_IN);
    gpio_pull_up(GPIO_BTN_JOY); // Habilita o pull-up para o botão do joystick
}

// Inicializa a comunicação I2C e o display SSD1306
void init_I2C() {
    i2c_init(I2C_PORT, 400 * 1000); // Inicializa I2C com 400kHz
    gpio_set_function(GPIO_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(GPIO_I2C_SLC, GPIO_FUNC_I2C); 
    gpio_pull_up(GPIO_I2C_SDA); 
    gpio_pull_up(GPIO_I2C_SLC); 
    ssd1306_init(&ssd, 128, 64, false, ADDRESS, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); 
    ssd1306_send_data(&ssd); 
    ssd1306_fill(&ssd, false); // Limpa a tela
    ssd1306_send_data(&ssd);

}

// Inicializa o ADC para ler os valores do joystick
void init_adc() {
    adc_init();
    adc_gpio_init(GPIO_VRX_JOY); // Inicializa o pino do eixo X do joystick
    adc_gpio_init(GPIO_VRY_JOY); // Inicializa o pino do eixo Y do joystick
}

// Inicializa o PWM para os LEDs
void init_pwm() {
    gpio_set_function(GPIO_LED_B, GPIO_FUNC_PWM); // Configura o pino para PWM
    gpio_set_function(GPIO_LED_R, GPIO_FUNC_PWM); // Configura o pino para PWM
    uint16_t slicePWMLedB = pwm_gpio_to_slice_num(GPIO_LED_B);
    uint16_t slicePWMLedR = pwm_gpio_to_slice_num(GPIO_LED_R);
    pwm_set_clkdiv(slicePWMLedB, PWM_DIVISER); // Define o divisor de clock para PWM
    pwm_set_clkdiv(slicePWMLedR, PWM_DIVISER); // Define o divisor de clock para PWM
    pwm_set_wrap(slicePWMLedB, wrapValue); // Define o valor máximo de contagem
    pwm_set_wrap(slicePWMLedR, wrapValue); // Define o valor máximo de contagem
    pwm_set_enabled(slicePWMLedB, 1); // Habilita o PWM no LED azul
    pwm_set_enabled(slicePWMLedR, 1); // Habilita o PWM no LED vermelho
}

// Função para mapear o valor de um intervalo para outro
int mapValue(int x, int in_min, int in_max, int out_min, int out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Posições iniciais do quadrado na tela
int lcdLeft = (128 / 2) - 4;
int lcdTop = (64 / 2) - 4;

int main() {
    stdio_init_all();
    init_leds();    // Inicializa os LEDs
    init_botoes();  // Inicializa os botões
    init_I2C();     // Inicializa a comunicação I2C
    init_adc();     // Inicializa o ADC
    init_pwm();     // Inicializa o PWM

    // Configura as interrupções para os botões
    gpio_set_irq_enabled_with_callback(GPIO_BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(GPIO_BTN_JOY, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    ssd1306_fill(&ssd, false); // Limpa a tela
    ssd1306_rect(&ssd, lcdTop, lcdLeft, 8, 8, true, true); // Desenha o quadrado
    ssd1306_send_data(&ssd);

    while (true) {
        adc_select_input(1); // Seleciona o canal do eixo X
        adc_valor_x = adc_read(); // Lê o valor do eixo X
        adc_select_input(0); // Seleciona o canal do eixo Y
        adc_valor_y = adc_read(); // Lê o valor do eixo Y

        // Mapeia os valores lidos para PWM
        int pwm_valor_x = mapValue(adc_valor_x, 0, 4095, 0, wrapValue);
        int pwm_valor_y = mapValue(adc_valor_y, 0, 4095, 0, wrapValue);

        // Mapeia as posições do joystick para o quadrado na tela
        lcdLeft = mapValue(adc_valor_x, 0, 4095, 3, 117);
        lcdTop = mapValue(adc_valor_y, 0, 4095, 56, 3);

        ssd1306_fill(&ssd, 0); // Limpa a tela
        sleep_ms(40);
        ssd1306_rect(&ssd, lcdTop, lcdLeft, 8, 8, 1, 1); // Desenha o quadrado
        ssd1306_rect(&ssd, 0, 0, 127, 63, true, false); // Desenha borda externa
        ssd1306_send_data(&ssd); // Atualiza o display
        sleep_ms(40);

        // Exibe os valores de PWM e ADC no terminal serial
        printf("JOYSTICK - PWM X = %d / PWM Y = %d\n", pwm_valor_x, pwm_valor_y);
        printf("JOYSTICK - ADC X = %d / ADC Y = %d\n", adc_valor_x, adc_valor_y);
        printf("QUADRADO - Top   = %d / Left  = %d\n", lcdTop, lcdLeft);

        // Controla os LEDs com base nos valores do joystick
        if (adc_valor_y >= 0 && adc_valor_y <= 2090 && pwm_leds_ativos) {
            pwm_set_gpio_level(GPIO_LED_B, 0); // Apaga o LED azul
        } else if (pwm_leds_ativos) {
            pwm_set_gpio_level(GPIO_LED_B, pwm_valor_y); // Ajusta o brilho do LED azul
        }

        if (adc_valor_x >= 0 && adc_valor_x <= 2045 && pwm_leds_ativos) {
            pwm_set_gpio_level(GPIO_LED_R, 0); // Apaga o LED vermelho
        } else if (pwm_leds_ativos) {
            pwm_set_gpio_level(GPIO_LED_R, pwm_valor_x); // Ajusta o brilho do LED vermelho
        }
    }

    return 0;
}
