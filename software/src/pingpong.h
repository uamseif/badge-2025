#pragma once
#include "gpio.h"

#define PING_PONG_LEDS 12
#define PING_PONG_STATE_START 0
#define PING_PONG_STATE_GAME 1
#define PING_PONG_STATE_FAIL 2
#define PING_PONG_STATE_WIN  3
#define PING_PONG_STATE_WAIT 4

#define MSG_ACTION_BOUNCE 0
#define MSG_ACTION_MASTER_WIN 1
#define MSG_ACTION_SLAVE_WIN 2

uint8_t leds[] = {1, 2, 3, 4, 5, 7, 12, 13, 14, 15, 16, 21};
int8_t current_led = 0;
static uint32_t pingMillis = 0;
uint8_t speed = 1;
int ms_per_speed = 1000;
uint8_t win_led = 8;
uint8_t direction = 1;
uint8_t current_state = PING_PONG_STATE_START;

uint8_t error_led = 7;
uint8_t success_led = 16;
uint8_t error_leds_count = 4;
uint8_t error_cycle = 0;
uint8_t error_cycles_count = 40;

extern void return_to_menu();
extern uint8_t multiplayer;


static inline void draw_ping_pong() {
    for (uint8_t i = 0; i < PING_PONG_LEDS; i++) {
        leds_buffer[leds[i] - 1] = current_led == i ? 1 : 0;
    }
}

static inline void pingpong_init() {
    clear_leds();
    pingMillis = systick_millis;

    if (multiplayer == MULTIPLAYER_SLAVE) {
        current_state = PING_PONG_STATE_WAIT;
    }
}

static inline void pingpong_error() {
    current_led = 0;
    speed = 1;
    clear_leds();
    current_state = PING_PONG_STATE_FAIL;
}

static inline void pingpong_win() {
    current_led = 0;
    speed = 1;
    clear_leds();
    current_state = PING_PONG_STATE_WIN;
}

static inline void pingpong_success() {
    speed += 1;
    direction = -1;
}

static inline void update_multiplayer() {
    if (multiplayer > 0) {
        const uint8_t pingpong_message[] = { multiplayer == MULTIPLAYER_MASTER ? MSG_ACTION_SLAVE_WIN : MSG_ACTION_MASTER_WIN };
        const uint8_t pingpong_message_length = sizeof(pingpong_message);
        send_game_message(MSG_GAME_PINGPONG, pingpong_message, pingpong_message_length);
    }
}

static inline void pingpong_game() {
    const uint32_t currentMillis = systick_millis;

    if (currentMillis - pingMillis >= MS_TO_TICKS(ms_per_speed / speed)) {

        pingMillis = currentMillis;

        current_led = (int8_t)(current_led + direction);
        if (current_led == PING_PONG_LEDS) {
            pingpong_error();
            update_multiplayer();
        }

        if (current_led < 0) {
            if (multiplayer > 0) {
                direction = 1;
                current_led = -1;
                current_state = PING_PONG_STATE_WAIT;
                clear_leds();

                uint8_t pingpong_message[] = { MSG_ACTION_BOUNCE, speed };
                uint8_t pingpong_message_length = sizeof(pingpong_message);

                send_game_message(MSG_GAME_PINGPONG, pingpong_message, pingpong_message_length);
            } else {
                direction = 1;
                current_led = 1;
            }

        }
    }

    if (button_is_pressed(BUTTON_D) || button_is_pressed(BUTTON_B)) {
        if (current_led >= win_led && current_led <= win_led + 1) {
            pingpong_success();
        } else {
            pingpong_error();
            update_multiplayer();
        }
    }
}

static inline void pingpong_fail_leds() {
    const uint32_t currentMillis = systick_millis;

    if (currentMillis - pingMillis >= MS_TO_TICKS(100)) {
        pingMillis = currentMillis;

        leds_buffer[error_led + current_led] = 0;
        current_led += 1;
        if (current_led >= error_leds_count) {
            current_led = 0;
        }
        leds_buffer[error_led + current_led] = 1;

        error_cycle += 1;
        if (error_cycle >= error_cycles_count) {
            if (multiplayer == MULTIPLAYER_SLAVE) {
                current_state = PING_PONG_STATE_WAIT;
            } else {
                current_state = PING_PONG_STATE_START;
            }
            error_cycle = 0;
            clear_leds();
        }
    }
}

static inline void pingpong_success_leds() {
    const uint32_t currentMillis = systick_millis;

    if (currentMillis - pingMillis >= MS_TO_TICKS(100)) {
        pingMillis = currentMillis;

        leds_buffer[success_led + current_led] = 0;
        current_led += 1;
        if (current_led >= error_leds_count) {
            current_led = 0;
        }
        leds_buffer[success_led + current_led] = 1;

        error_cycle += 1;
        if (error_cycle >= error_cycles_count) {
            current_state = multiplayer == MULTIPLAYER_MASTER ? PING_PONG_STATE_START : PING_PONG_STATE_WAIT;
            error_cycle = 0;
            clear_leds();
        }
    }
}

static inline void pingpong_run() {
    switch (current_state) {
        case PING_PONG_STATE_START:
            current_state = PING_PONG_STATE_GAME;
            current_led = 0;
            speed = 1;
            break;
        case PING_PONG_STATE_GAME:
            draw_ping_pong();
            pingpong_game();

            break;
        case PING_PONG_STATE_FAIL:
            pingpong_fail_leds();
            break;
        case PING_PONG_STATE_WIN:
            pingpong_success_leds();
        break;
        case PING_PONG_STATE_WAIT:
            if (multiplayer_game_data != NULL) {
                const uint8_t game_id = multiplayer_game_data[0];
                if (game_id == MSG_GAME_PINGPONG) {
                    const uint8_t action = multiplayer_game_data[1];
                    switch (action) {
                        case MSG_ACTION_BOUNCE:
                            speed = multiplayer_game_data[2];;
                            direction = 1;
                            current_led = -1;
                            current_state = PING_PONG_STATE_GAME;
                            break;
                        case MSG_ACTION_MASTER_WIN:
                            if (multiplayer == MULTIPLAYER_SLAVE) {
                                pingpong_error();
                            } else if (multiplayer == MULTIPLAYER_MASTER) {
                                pingpong_win();
                            }
                            break;
                        case MSG_ACTION_SLAVE_WIN:
                            if (multiplayer == MULTIPLAYER_MASTER) {
                                pingpong_error();
                            } else if (multiplayer == MULTIPLAYER_SLAVE) {
                                pingpong_win();
                            }
                            break;
                        default: break;
                    }
                }
                multiplayer_game_data = NULL;
            }
        default: break;
    }

    if (button_is_pressed(BUTTON_MENU)) {
        return_to_menu();
    }
}