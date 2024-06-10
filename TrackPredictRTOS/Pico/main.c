#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include "pico/mutex.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define PWM_PIN1 2             // GP2 pin for PWM output (Servo 1)
#define PWM_PIN2 4             // GP4 pin for PWM output (Servo 2)
#define PWM_FREQ 50            // PWM frequency in Hz
#define MIN_PULSE_WIDTH 500    // Minimum pulse width in microseconds
#define MAX_PULSE_WIDTH 2500   // Maximum pulse width in microseconds
#define PWM_WRAP 20000         // 20 ms period (50 Hz)

#define BUZZER_CORE1 16 // buzzer
#define LED_B 6 // buzzer led blue
#define LASER  18 //laser
#define LED_PIN_ON  19 //on
#define LED_PIN_OFF  20 // off

volatile unsigned char shared_conf = 0;

void core1_main() {
    gpio_init(BUZZER_CORE1);
    gpio_set_dir(BUZZER_CORE1, GPIO_OUT);

    while (true) {
        int confidence = shared_conf;
        if (confidence < 150 && confidence >= 40) { confidence = 40; }
        else if (confidence < 40) {
            gpio_put(BUZZER_CORE1, 1);
            gpio_put(LED_B, 1);
            sleep_ms(40);
            continue;
        }
        else if(confidence == -1){
            gpio_put(BUZZER_CORE1, 0);
            gpio_put(LED_B, 0);
        }
	else{
            gpio_put(BUZZER_CORE1, 1);
            gpio_put(LED_B, 1);
            sleep_ms(confidence);
            gpio_put(BUZZER_CORE1, 0);
            gpio_put(LED_B, 0);
            sleep_ms(confidence);
        }
    }
}

void init_uart() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

void init_pwm(uint gpio_pin) {
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio_pin);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 64.0f);
    pwm_config_set_wrap(&config, PWM_WRAP);
    pwm_init(slice_num, &config, true);

    printf("Initialized PWM on GPIO %d, slice %d\n", gpio_pin, slice_num);
}

void set_servo_angle(uint gpio_pin, float angle) {
    float pulse_width_us = (angle / 180.0f) * (MAX_PULSE_WIDTH - MIN_PULSE_WIDTH) + MIN_PULSE_WIDTH;
    uint16_t pulse_width = (uint16_t)((pulse_width_us / 20000.0f) * PWM_WRAP);
    pwm_set_gpio_level(gpio_pin, pulse_width);
    printf("Set GPIO %d to angle %.2f, pulse width %u\n", gpio_pin, angle, pulse_width);
}

void uart_flush() {
    while (uart_is_readable(UART_ID)) {
        uart_getc(UART_ID);
    }
}


void read_uart_data(uint16_t* buffer, size_t len) {
    uint8_t byte_buffer[6];

    //rread exactly len * 2 bytes
    uart_read_blocking(UART_ID, byte_buffer, len * 2);

    //combine bytes into uint16_t values
    for (size_t i = 0; i < len; i++) {
        buffer[i] = (byte_buffer[2 * i] << 8) | byte_buffer[2 * i + 1];
    }
    uart_flush();
}

int main() {
    stdio_init_all();
    init_uart();
    init_pwm(PWM_PIN1);
    init_pwm(PWM_PIN2);

    uint slice_num1 = pwm_gpio_to_slice_num(PWM_PIN1);
    uint slice_num2 = pwm_gpio_to_slice_num(PWM_PIN2);

    printf("Slice numbers: PWM_PIN1 = %d, PWM_PIN2 = %d\n", slice_num1, slice_num2);

    multicore_launch_core1(core1_main);
    
    gpio_init(LED_PIN_ON);
    gpio_set_dir(LED_PIN_ON, GPIO_OUT);
    gpio_init(LED_PIN_OFF);
    gpio_set_dir(LED_PIN_OFF, GPIO_OUT);
    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);
    
    gpio_init(LASER);
    gpio_set_dir(LASER, GPIO_OUT);
    gpio_put(LASER, 1);
    
    bool led_state = false;
    bool led_toggled = false;

    while (1) {
        if (uart_is_readable(UART_ID)) {
            uint16_t rx_buffer[3];
            read_uart_data(rx_buffer, 3);

            uint16_t angle1 = rx_buffer[0];
            uint16_t angle2 = rx_buffer[1];
            uint16_t conf = rx_buffer[2];

            shared_conf = conf;

            printf("Received data: angle1 = %d, angle2 = %d, conf = %d\n", angle1, angle2, conf);

            set_servo_angle(PWM_PIN1, angle1);
            set_servo_angle(PWM_PIN2, angle2);
            
            if((conf == -1 || conf == 65535) && !led_toggled){
                    led_state = !led_state;
                    led_toggled = true;
                } else {
                    led_toggled = false;
            }
            
            if(!led_state){
                gpio_put(LED_PIN_OFF, 1);
                gpio_put(LED_PIN_ON, 0);
                }
            else{
                gpio_put(LED_PIN_ON, 1);
                gpio_put(LED_PIN_OFF, 0);
            }
        }
        sleep_ms(40);
    }

    return 0;
}

