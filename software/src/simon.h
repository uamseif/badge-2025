#pragma once
#include "gpio.h"
#include "lib_rand.h"

#define STATE_SEQUENCE      0
#define STATE_USER_INPUT    1
#define STATE_PLAY_GUESS    2
#define STATE_PLAY_ERROR    3
#define STATE_PLAY_SUCCESS  4

#define SEQUENCE_LENGTH 100
#define LED_ON_TIME 1000
#define LED_OFF_TIME 400
#define LEDS_PER_SEQUENCE 4
#define INFO_SEQUENCE_COUNT 20

static uint32_t sequence[SEQUENCE_LENGTH];
static uint8_t sequence_index = 0;
static uint8_t player_index = 0;
static uint8_t player_guess = 0;


static uint8_t led_state = 0;
static uint8_t button_guessed = 0;

uint8_t sequence_leds[] = {7, 16, 21, 25};

uint8_t success_leds[] = {0, 1};
uint8_t error_leds[] = {15, 20};
uint8_t info_count = 0;

uint8_t simon_buttons[] = {BUTTON_C, BUTTON_D, BUTTON_B, BUTTON_A};

uint8_t score_leds_simon[] = {2, 3, 5, 4, 29, 6, 11, 12, 13, 14};
static uint8_t score_count_simon = 0;

static uint8_t simon_state = STATE_SEQUENCE;

static inline void update_score_leds_simon() {
    for (uint8_t i = 0; i < 10; i++) {
        leds_buffer[score_leds_simon[i]] = (i < score_count_simon) ? 1 : 0;
    }
}


static inline void generate_sequence() {
    seed(systick_millis);

    uint8_t last_value = 255;
    uint8_t repeat_count = 0;

    for (uint8_t i = 0; i < SEQUENCE_LENGTH; i++) {
        uint8_t next_value;

        do {
            next_value = rand() % 4;
        } while (repeat_count >= 3 && next_value == last_value);

        sequence[i] = next_value;

        if (next_value == last_value) {
            repeat_count++;
        } else {
            repeat_count = 1;
            last_value = next_value;
        }
    }

    sequence_index = 0;
    player_index = 0;
    simon_state = STATE_SEQUENCE;
    last_millis = systick_millis;
    led_state = 0;
}

static inline void simon_init() {
    clear_leds();
    generate_sequence();
    score_count_simon = 0;
    update_score_leds_simon();
}

static inline void play_sequence() {
    const uint32_t current_millis = systick_millis;

    if (player_index >= SEQUENCE_LENGTH) {
        simon_init();
        return;
    }

    if (led_state == 0 && (current_millis - last_millis >= MS_TO_TICKS(LED_OFF_TIME))) {
        for (uint8_t i = 0; i < LEDS_PER_SEQUENCE; i++) {
            leds_buffer[sequence_leds[sequence[sequence_index]] + i] = 1;
        }
        last_millis = current_millis;
        led_state = 1;
    }
    else if (led_state == 1 && (current_millis - last_millis >= MS_TO_TICKS(LED_ON_TIME))) {
        for (uint8_t i = 0; i < LEDS_PER_SEQUENCE; i++) {
            leds_buffer[sequence_leds[sequence[sequence_index]] + i] = 0;
        }
        last_millis = current_millis;
        led_state = 0;
        sequence_index++;

        if (sequence_index > player_index) {
            simon_state = STATE_USER_INPUT;
            sequence_index = 0;
        }
    }
}

static inline void play_guess() {
    const uint32_t current_millis = systick_millis;
    if (led_state == 0) {
        for (uint8_t i = 0; i < LEDS_PER_SEQUENCE; i++) {
            leds_buffer[sequence_leds[button_guessed] + i] = 1;
        }
        last_millis = current_millis;
        led_state = 1;
    }
    else if (led_state == 1 && (current_millis - last_millis >= MS_TO_TICKS(300))) {
        for (uint8_t i = 0; i < LEDS_PER_SEQUENCE; i++) {
            leds_buffer[sequence_leds[button_guessed] + i] = 0;
        }
        last_millis = current_millis;
        led_state = 0;
        simon_state = STATE_USER_INPUT;
    }
}

static inline void guess_correct() {
    player_index++;

    if (player_index > SEQUENCE_LENGTH) {
        simon_init();
        return;
    }

    score_count_simon = player_index % 10;
    if (score_count_simon == 0) { 
        score_count_simon = 10;
    }

    update_score_leds_simon(); 

    simon_state = STATE_PLAY_SUCCESS;
    player_guess = 0;
}


static inline void play_success() {
    const uint32_t current_millis = systick_millis;

    const uint8_t state = leds_buffer[success_leds[0]] == 1;
    if ((current_millis - last_millis >= MS_TO_TICKS(100))) {
        leds_buffer[success_leds[0]] = state == 1 ? 0 : 1;
        leds_buffer[success_leds[1]] = state == 1 ? 1 : 0;
        last_millis = current_millis;

        info_count++;
        if (info_count > INFO_SEQUENCE_COUNT) {
            simon_state = STATE_SEQUENCE;
            clear_leds();
            info_count = 0;
        }
    }
}

static inline void play_error() {
    const uint32_t current_millis = systick_millis;

    const uint8_t state = leds_buffer[error_leds[0]] == 1;
    if ((current_millis - last_millis >= MS_TO_TICKS(100))) {
        leds_buffer[error_leds[0]] = state == 1 ? 0 : 1;
        leds_buffer[error_leds[1]] = state == 1 ? 1 : 0;
        last_millis = current_millis;

        info_count++;
        if (info_count > INFO_SEQUENCE_COUNT) {
            simon_init();
            score_count_simon = 0;
            update_score_leds_simon();
            info_count = 0;
            clear_leds();
        }
    }
}

static inline void simon_user_input() {
    if (player_guess <= player_index) {
        for (uint8_t i = 0; i < LEDS_PER_SEQUENCE; i++) {
            if (button_is_pressed(simon_buttons[i])) {
                if (sequence[player_guess] == i) {
                    button_guessed = i;
                    player_guess++;
                    simon_state = STATE_PLAY_GUESS;
                } else {
                    simon_state = STATE_PLAY_ERROR;
                }
            }
        }
    } else {
        if (player_index >= SEQUENCE_LENGTH) {
            simon_init();
        } else {
            guess_correct();
        }
    }
}

static inline void simon_run() {
    switch (simon_state) {
        case STATE_SEQUENCE:
            play_sequence();
            break;
        case STATE_USER_INPUT:
            simon_user_input();
            break;
        case STATE_PLAY_GUESS:
            play_guess();
            break;
        case STATE_PLAY_ERROR:
            play_error();
            break;
        case STATE_PLAY_SUCCESS:
            play_success();
            break;
        default:
            break;
    }

    if (button_is_pressed(BUTTON_MENU)) {
        return_to_menu();
    }
}
