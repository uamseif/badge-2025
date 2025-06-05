#pragma once

#include "ch32v003_GPIO_branchless.h"
#include "definitions.h"
#include "lib_uart.h"

#define MS_TO_TICKS(ms) ((ms) * 48)

#define LED_MATRIX_NUM_PINS     6
#define LED_MATRIX_SIZE     (LED_MATRIX_NUM_PINS * (LED_MATRIX_NUM_PINS - 1))
#define BUTTON_COUNT            5

#define PIN_BEEP             PC7

#define PIN_BUTTON_A         PD4
#define PIN_BUTTON_B         PD0
#define PIN_BUTTON_C         PD3
#define PIN_BUTTON_D         PC0
#define PIN_BUTTON_MENU      PD2

#define BUTTON_A             0
#define BUTTON_B             1
#define BUTTON_C             2
#define BUTTON_D             3
#define BUTTON_MENU          4
#define DEBOUNCE_DELAY_MS    2000

#define UART_BUFFER_SIZE 32
uint8_t uart_buffer[UART_BUFFER_SIZE];
size_t uart_bytes_read;

static uint32_t last_millis = 0;

uint8_t leds_buffer[LED_MATRIX_SIZE];
#define BUZZER_BUFFER_SIZE 10
Sound sound_buffer[BUZZER_BUFFER_SIZE];
uint8_t buzzer_head = 0, buzzer_tail = 0, buzzer_count = 0, buzzer_state = 0;
static uint32_t buzzer_millis = 0;

Button buttons[BUTTON_COUNT];

extern volatile uint32_t systick_millis;

int led_positions[LED_MATRIX_SIZE][2] = {
    {PC1, PC2}, // LED 1
    {PC1, PC3}, // LED 2
    {PC1, PC4}, // LED 3
    {PC1, PC5}, // LED 4
    {PC1, PC6}, // LED 5

    {PC2, PC1}, // LED 6
    {PC2, PC3}, // LED 7
    {PC2, PC4}, // LED 8
    {PC2, PC5}, // LED 9
    {PC2, PC6}, // LED 10

    {PC3, PC1}, // LED 11
    {PC3, PC2}, // LED 12
    {PC3, PC4}, // LED 13
    {PC3, PC5}, // LED 14
    {PC3, PC6}, // LED 15

    {PC4, PC1}, // LED 16
    {PC4, PC2}, // LED 17
    {PC4, PC3}, // LED 18
    {PC4, PC5}, // LED 19
    {PC4, PC6}, // LED 20

    {PC5, PC1}, // LED 21
    {PC5, PC2}, // LED 22
    {PC5, PC3}, // LED 23
    {PC5, PC4}, // LED 24
    {PC5, PC6}, // LED 25

    {PC6, PC1}, // LED 26
    {PC6, PC2}, // LED 27
    {PC6, PC3}, // LED 28
    {PC6, PC4}, // LED 29
    {PC6, PC5}, // LED 30
};

static inline void led_matrix_init()
{
    GPIO_port_enable(GPIO_port_C);
}

static inline void led_matrix_run()
{
    static int ledIndex = 0;


    GPIO_pinMode(led_positions[ledIndex][0], GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
    GPIO_pinMode(led_positions[ledIndex][1], GPIO_pinMode_I_floating, GPIO_Speed_10MHz);

    ledIndex++;

    if (ledIndex >= LED_MATRIX_SIZE) {
        ledIndex = 0;
    }

    const uint8_t led = leds_buffer[ledIndex];

    if (led) {
        GPIO_pinMode(led_positions[ledIndex][0], GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
        GPIO_pinMode(led_positions[ledIndex][1], GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);

        GPIO_digitalWrite(led_positions[ledIndex][0], low);
        GPIO_digitalWrite(led_positions[ledIndex][1], high);
    }

}

static inline void clear_leds() {
    for (int i = 0; i < LED_MATRIX_SIZE; i++) {
        leds_buffer[i] = 0;
    }
}

static inline void buzzer_init() {
    GPIO_port_enable(GPIO_port_C);

}

static inline void add_sound(uint32_t frequency, uint32_t duration) {
    if (buzzer_count < BUZZER_BUFFER_SIZE) {
        sound_buffer[buzzer_tail].frequency = frequency;
        sound_buffer[buzzer_tail].duration = duration;
        buzzer_tail = (buzzer_tail + 1) % BUZZER_BUFFER_SIZE;
        buzzer_count++;
    } else {
#ifdef DEBUG
        printf("Buffer lleno, no se puede agregar sonido.\n");
#endif
    }
}


static inline void buzzer_run() {
    const uint32_t currentMillis = systick_millis;
    if (buzzer_count > 0) {
        if (currentMillis - buzzer_millis >= sound_buffer[buzzer_head].frequency) {
            buzzer_state = buzzer_state  == 0 ? 1 : 0;
            buzzer_millis = currentMillis;
            if (buzzer_state == 1) {
                GPIO_pinMode(PIN_BEEP, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
            } else {
                GPIO_pinMode(PIN_BEEP, GPIO_pinMode_I_floating, GPIO_Speed_10MHz);
            }
        }
        sound_buffer[buzzer_head].duration--;

        if (sound_buffer[buzzer_head].duration <= 0) {
            buzzer_head = (buzzer_head + 1) % BUZZER_BUFFER_SIZE;
            buzzer_count--;
        }
    }
}

static inline int button_is_pressed(const uint8_t btn) {
    Button *button = &buttons[btn];
    const int currentState = GPIO_digitalRead(button->pin) == 1;

    uint32_t now = systick_millis;

    if (currentState != button->lastState) {
        button->lastDebounceTime = now;
    }

    if ((now - button->lastDebounceTime) > DEBOUNCE_DELAY_MS) {
        if (currentState != button->debouncedState) {
            button->debouncedState = currentState;

            if (button->debouncedState == 0) {
                button->lastState = currentState;
                return 1;
            }
        }
    }

    button->lastState = currentState;
    return 0;
}

static inline void buttons_init() {
    GPIO_port_enable(GPIO_port_D);
    GPIO_port_enable(GPIO_port_C);

    buttons[BUTTON_A].pin = PIN_BUTTON_A;
    buttons[BUTTON_B].pin = PIN_BUTTON_B;
    buttons[BUTTON_C].pin = PIN_BUTTON_C;
    buttons[BUTTON_D].pin = PIN_BUTTON_D;
    buttons[BUTTON_MENU].pin = PIN_BUTTON_MENU;

    for (int i = 0; i < BUTTON_COUNT; i++) {
        GPIO_pinMode(buttons[i].pin, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    }

}


static inline void gpio_uart_init() {
    // Create a UART Configuration
    uart_config_t uart_conf = {
        .baudrate    = UART_BAUD_9600,
        .wordlength  = UART_WORDLENGTH_8,
        .parity      = UART_PARITY_NONE,
        .stopbits    = UART_STOPBITS_ONE,
        .flowctrl    = UART_FLOWCTRL_NONE,
    };

    uart_err_t err_state;

    // Initialise the UART
    err_state = uart_init(uart_buffer, UART_BUFFER_SIZE, &uart_conf );
    if(err_state != UART_OK)
    {
#ifdef DEBUG
        printf("Failed to Initialise the UART\n");
#endif
    }
}

static inline void gpio_uart_read() {
    uart_bytes_read = uart_read(uart_buffer, UART_BUFFER_SIZE);
}