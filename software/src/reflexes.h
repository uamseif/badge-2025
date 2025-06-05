#pragma once
#include "gpio.h"
#include "lib_rand.h"

#define STATE_WAIT_FOR_START    0
#define STATE_GENERATE_LIGHT    1
#define STATE_WAIT_FOR_BUTTON   2
#define STATE_SUCCESS           3
#define STATE_TIMEOUT_OR_ERROR  4

#define SUCCESS_ERROR_BLINK_COUNT 10
#define REACTION_TIME_START_MS    2000
#define REACTION_TIME_MIN_MS      500
#define REACTION_TIME_STEP_MS     100

#define MSG_ACTION_SET_BUTTON 0
#define MSG_ACTION_MASTER_WIN 1
#define MSG_ACTION_SLAVE_WIN 2

static uint8_t current_target_led_index = 0;
static uint8_t blink_count = 0;
static uint16_t current_reaction_time_ms = REACTION_TIME_START_MS;

uint8_t reaction_leds[] = {7, 16, 21, 25};
uint8_t reaction_buttons[] = {BUTTON_C, BUTTON_D, BUTTON_B, BUTTON_A};
uint8_t success_leds_reaction[] = {0, 1};
uint8_t error_leds_reaction[] = {15, 20};

uint8_t score_leds[] = {2, 3, 5, 4, 29, 6, 11, 12, 13, 14};

uint8_t multiplayer_turn_led = 20;

static uint8_t score_count = 0;
static uint8_t score_blinking = 0;

static uint8_t game_state = STATE_WAIT_FOR_START;

bool my_turn = false;


static inline void update_score_leds() {
    for (uint8_t i = 0; i < 10; i++) {
        leds_buffer[score_leds[i]] = (i < score_count) ? 1 : 0;
    }
}

static inline void blink_score_leds() {
    const uint32_t current_millis = systick_millis;
    if ((current_millis - last_millis) >= MS_TO_TICKS(100)) {
        for (uint8_t i = 0; i < 10; i++) {
            leds_buffer[score_leds[i]] ^= 1;
        }

        last_millis = current_millis;
        blink_count++;

        if (blink_count >= SUCCESS_ERROR_BLINK_COUNT) {
            score_blinking = 0;
            blink_count = 0;
            score_count = 0;
            update_score_leds();
            game_state = STATE_WAIT_FOR_START;
        }
    }
}

static inline void reaction_init() {
    clear_leds();
    game_state = STATE_WAIT_FOR_START;
    last_millis = systick_millis;
    blink_count = 0;
    current_reaction_time_ms = REACTION_TIME_START_MS;
    score_count = 0;
    update_score_leds();
    if (multiplayer == MULTIPLAYER_MASTER) {
        my_turn = true;
    }
}

static inline void show_reaction_leds() {
    for (uint8_t i = 0; i < 4; i++) {
        leds_buffer[reaction_leds[current_target_led_index] + i] = 1;
    }

    last_millis = systick_millis;
    game_state = STATE_WAIT_FOR_BUTTON;
}

static inline void generate_random_light() {
    seed(systick_millis);
    current_target_led_index = rand() % 4;

    show_reaction_leds();
}

static inline void handle_success() {
    if (score_blinking) {
        blink_score_leds();
        return;
    }

    const uint32_t current_millis = systick_millis;

    if ((current_millis - last_millis) >= MS_TO_TICKS(100)) {
        uint8_t state = leds_buffer[success_leds_reaction[0]] == 1 ? 0 : 1;
        leds_buffer[success_leds_reaction[0]] = state;
        leds_buffer[success_leds_reaction[1]] = state == 1 ? 0 : 1;

        last_millis = current_millis;
        blink_count++;

        if (blink_count >= SUCCESS_ERROR_BLINK_COUNT) {
            clear_leds();
            blink_count = 0;

            if (score_count < 10) {
                score_count++;
                update_score_leds();
            }

            if (current_reaction_time_ms > REACTION_TIME_MIN_MS) {
                current_reaction_time_ms -= REACTION_TIME_STEP_MS;
                if (current_reaction_time_ms < REACTION_TIME_MIN_MS) {
                    current_reaction_time_ms = REACTION_TIME_MIN_MS;
                }
            }

            if (score_count >= 10) {
                score_blinking = 1;
                game_state = STATE_SUCCESS;
            } else {
                game_state = STATE_WAIT_FOR_START;
            }
        }
    }
}

static inline void handle_timeout_or_error() {
    const uint32_t current_millis = systick_millis;

    if ((current_millis - last_millis) >= MS_TO_TICKS(100)) {
        uint8_t state = leds_buffer[error_leds_reaction[0]] == 1 ? 0 : 1;
        leds_buffer[error_leds_reaction[0]] = state;
        leds_buffer[error_leds_reaction[1]] = state == 1 ? 0 : 1;

        last_millis = current_millis;
        blink_count++;

        if (blink_count >= SUCCESS_ERROR_BLINK_COUNT) {
            reaction_init();
        }
    }

    score_count = 0;
    update_score_leds();
}

static inline void clear_reflexes_leds() {
    for (uint8_t i = 0; i < 4; i++) {
        for (uint8_t j = 0; j < 4; j++) {
            leds_buffer[reaction_leds[j] + i] = 0;
        }
    }
}

static inline void handle_wait_for_button() {
    const uint32_t current_millis = systick_millis;

    if ((current_millis - last_millis) >= MS_TO_TICKS(current_reaction_time_ms)) {
        clear_reflexes_leds();
        game_state = STATE_TIMEOUT_OR_ERROR;
        blink_count = 0;
        last_millis = current_millis;
        return;
    }

    for (uint8_t i = 0; i < 4; i++) {
        if (button_is_pressed(reaction_buttons[i])) {
            if (i == current_target_led_index) {
                game_state = STATE_SUCCESS;
            } else {
                game_state = STATE_TIMEOUT_OR_ERROR;
            }
            clear_reflexes_leds();
            blink_count = 0;
            last_millis = current_millis;
            return;
        }
    }
}

static inline void handle_wait_for_multiplayer_generation() {
    leds_buffer[multiplayer_turn_led] = 1;
    for (uint8_t i = 0; i < 4; i++) {
        if (button_is_pressed(reaction_buttons[i])) {
            uint8_t msg[] = { MSG_ACTION_SET_BUTTON, i };
            send_game_message(MSG_GAME_REFLEXES, msg, sizeof(msg));
            my_turn = false;
            leds_buffer[multiplayer_turn_led] = 0;
        }
    }
}

static inline void reaction_run() {
    switch (game_state) {
        case STATE_WAIT_FOR_START:
            if (multiplayer == MULTIPLAYER_OFF) {
                game_state = STATE_GENERATE_LIGHT;
            } else if (my_turn) {
                handle_wait_for_multiplayer_generation();

            } else {
                if (multiplayer_game_data != NULL) {
                    const uint8_t game_id = multiplayer_game_data[0];
                    if (game_id == MSG_GAME_REFLEXES) {
                        const uint8_t action = multiplayer_game_data[1];
                        switch (action) {
                            case MSG_ACTION_SET_BUTTON:
                                current_target_led_index = multiplayer_game_data[2];
                                show_reaction_leds();
                                my_turn = true;
                                break;
                            default: ;
                        }
                    }
                    multiplayer_game_data = NULL;
                }
            }
            break;

        case STATE_GENERATE_LIGHT:
            generate_random_light();
            break;

        case STATE_WAIT_FOR_BUTTON:
            handle_wait_for_button();
            break;

        case STATE_SUCCESS:
            handle_success();
            break;

        case STATE_TIMEOUT_OR_ERROR:
            handle_timeout_or_error();
            break;
        default: ;
    }

    if (button_is_pressed(BUTTON_MENU)) {
        return_to_menu();
    }
}
