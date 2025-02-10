#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "uart_usb.pio.h"

#define LED_VERDE 11
#define LED_AZUL 12
#define BOTAO_A 5
#define BOTAO_B 6
#define LED_MATRIZ 7
#define RGBW_FLAG false
#define TOTAL_PIXELS 25
#define I2C_CANAL i2c1
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15
#define ENDERECO_I2C 0x3C

bool digitos[10][TOTAL_PIXELS] = {
    { 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0 },
    { 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0 },
    { 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0 },
    { 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0 },
    { 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0 }
};

uint32_t buffer_leds[TOTAL_PIXELS] = {0};
uint8_t digito_atual = 0;

static inline void exibir_pixel(uint32_t cor_pixel) {
    pio_sm_put_blocking(pio0, 0, cor_pixel << 8u);
}

static inline uint32_t criar_cor(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void atualizar_buffer_leds() {
    for (int i = 0; i < TOTAL_PIXELS; i++) {
        buffer_leds[i] = digitos[digito_atual][i] ? criar_cor(90, 0, 70) : 0;
    }
}

void aplicar_leds() {
    for (int i = 0; i < TOTAL_PIXELS; i++) {
        exibir_pixel(buffer_leds[i]);
    }
}

ssd1306_t tela;

int main() {
    stdio_init_all();
    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_put(LED_VERDE, 0);
    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_put(LED_AZUL, 0);
    
    i2c_init(I2C_CANAL, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    
    ssd1306_init(&tela, WIDTH, HEIGHT, false, ENDERECO_I2C, I2C_CANAL);
    ssd1306_config(&tela);
    ssd1306_send_data(&tela);
    ssd1306_fill(&tela, false);
    ssd1306_send_data(&tela);
    
    while (true) {
        if (stdio_usb_connected()) {
            char entrada;
            if (scanf("%c", &entrada) == 1) {
                printf("Recebido: '%c'\n", entrada);
                ssd1306_fill(&tela, false);
                
                if (entrada >= '0' && entrada <= '9') {
                    digito_atual = entrada - '0';
                    atualizar_buffer_leds();
                    aplicar_leds();
                    char exibir[2] = {entrada, '\0'};
                    ssd1306_draw_string(&tela, exibir, 40, 25);
                    ssd1306_send_data(&tela);
                }
            }
        }
        sleep_ms(40);
    }
    return 0;
}
