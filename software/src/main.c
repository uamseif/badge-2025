#include <stdio.h>
#include "ch32v003fun.h"
#include "gpio.h"
#include "multiplayer.h"
#include "pingpong.h"
#include "menu.h"
#include "simon.h"
#include "reflexes.h"


#define LOOP_STATE_MENU 0
#define LOOP_STATE_PINGPONG 1
#define LOOP_STATE_SIMON 2 
#define LOOP_STATE_REACTION 3

uint8_t ledsBuffer[30];
volatile uint32_t systick_millis;
uint8_t loop_state = LOOP_STATE_MENU;
uint8_t multiplayer = MULTIPLAYER_OFF;
uint8_t multiplayer_accepted_game = 0;

void systick_init(void)
{
    // Disable default SysTick behavior
    SysTick->CTLR = 0;

    // Enable the SysTick IRQ
    NVIC_EnableIRQ(SysTicK_IRQn);

    // Set the tick interval
    SysTick->CMP = (FUNCONF_SYSTEM_CORE_CLOCK / 50000) - 1;

    // Start at zero
    SysTick->CNT = 0;

    // Enable SysTick counter, IRQ, HCLK/1
    SysTick->CTLR = SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE | SYSTICK_CTLR_STCLK;
}


// SysTick ISR just counts ticks
__attribute__((interrupt)) void SysTick_Handler(void)
{
    // Move the compare further ahead in time. as a warning, if more than this length of time
    // passes before triggering, you may miss your interrupt.
    SysTick->CMP += (FUNCONF_SYSTEM_CORE_CLOCK / 50000);

    // Clear IRQ
    SysTick->SR = 0;
    systick_millis++;
    led_matrix_run();
}

void return_to_menu() {
    clear_leds();
    loop_state = LOOP_STATE_MENU;
}

void start_ping_pong() {
    loop_state = LOOP_STATE_PINGPONG;
    pingpong_init();
    menu_selection = 0;
}

void start_simon() {
    loop_state = LOOP_STATE_SIMON;
    simon_init();
    menu_selection = 0;
}

void start_reflexes() {
    loop_state = LOOP_STATE_REACTION;
    reaction_init();
    menu_selection = 0;
}

int main() {
    SystemInit();
    Delay_Ms(100);

    // Init systick
    systick_init();

    // Init LED matrix
    led_matrix_init();
    buttons_init();
    gpio_uart_init();

    //pingpong_init();
    menu_init();
    // ReSharper disable once CppDFAEndlessLoop
    while (1)
    {
        switch (loop_state) {
            case LOOP_STATE_MENU:
                menu_run();
                if (menu_selection == 1) {
                    start_ping_pong();
                } else if (menu_selection == 2) {
                    start_simon();
                } else if (menu_selection == 3) {
                    start_reflexes();
                } else if (menu_selection == 4) {
                    start_multiplayer_game(MSG_GAME_PINGPONG);
                    multiplayer = MULTIPLAYER_MASTER;
                    start_ping_pong();
                } else if (menu_selection == 5) {
                    start_multiplayer_game(MSG_GAME_REFLEXES);
                    multiplayer = MULTIPLAYER_MASTER;
                    start_reflexes();;
                }
                break;
            case LOOP_STATE_PINGPONG:
                pingpong_run();
                break;
            case LOOP_STATE_SIMON:
                simon_run();
                break;
            case LOOP_STATE_REACTION:
                reaction_run();
                break;    
            default:
                break;
        }
        process_uart();

        if (multiplayer_accepted_game != 0) {
            multiplayer = MULTIPLAYER_SLAVE;
            switch (multiplayer_accepted_game) {
                case MSG_GAME_PINGPONG:
                    start_ping_pong();
                    break;
                case MSG_GAME_REFLEXES:
                    start_reflexes();
                    break;
                case MSG_GAME_SIMON:
                default: break;
            }
            multiplayer_accepted_game = 0;
        }
    }
}
