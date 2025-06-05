#pragma once
#include <gpio.h>

#define MENU_OPTIONS 5
#define MENU_ITEM_STARTUP 0
#define MENU_ITEM_ITEM_SELECTION 1
#define MENU_ITEM_EXIT_GAME 2

uint8_t menu_led = 3;
int8_t current_option = 1;
uint8_t menu_state = MENU_ITEM_STARTUP;
static uint32_t ledsMillis = 0;
uint8_t leds_dance_current_led = 0;

uint8_t menu_selection = 0;

static inline void draw_menu() {

}

static inline void menu_item_selection() {
    for (int8_t i = 0; i < 4; i++) {
        leds_buffer[i] = (current_option >> (3 - i)) & 1;
    }

    if (button_is_pressed(BUTTON_D)) {
        current_option--;
    }

    if (button_is_pressed(BUTTON_C)) {
        current_option++;
    }

    if (button_is_pressed(BUTTON_B)) {
        menu_selection = current_option;
        menu_state = MENU_ITEM_STARTUP;
    }

    if (current_option > MENU_OPTIONS) {
        current_option = 1;
    } else if (current_option <= 0) {
        current_option = MENU_OPTIONS;
    }
}

static inline void menu_leds_dance() {
    uint32_t currentMillis = systick_millis;

    if (currentMillis - ledsMillis >= MS_TO_TICKS(50)) {

        ledsMillis = currentMillis;
        leds_buffer[leds_dance_current_led] = 0;
        leds_dance_current_led++;
        leds_buffer[leds_dance_current_led] = 1;

        if (leds_dance_current_led >= LED_MATRIX_SIZE) {
            leds_dance_current_led = 0;
        }
    }

    if (button_is_pressed(BUTTON_A)
        || button_is_pressed(BUTTON_B)
        || button_is_pressed(BUTTON_C)
        || button_is_pressed(BUTTON_D)
        || button_is_pressed(BUTTON_MENU)) {
        leds_dance_current_led = 0;
        menu_state = MENU_ITEM_ITEM_SELECTION;
        clear_leds();
    }
}


static inline void menu_init() {
    //menu_item_selection();
}

static inline void menu_run() {
    switch (menu_state) {
      case MENU_ITEM_STARTUP:
        //
        menu_leds_dance();
        break;
      case MENU_ITEM_ITEM_SELECTION:
        menu_item_selection();
        break;
      case MENU_ITEM_EXIT_GAME:
        break;
    default: break;
    }
}