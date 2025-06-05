#pragma once

//#define DEBUG

#define MULTIPLAYER_OFF 0
#define MULTIPLAYER_SLAVE 1
#define MULTIPLAYER_MASTER 2

#define MSG_GAME_SIMON 0x01
#define MSG_GAME_PINGPONG 0x02
#define MSG_GAME_REFLEXES 0x03

typedef struct {
    uint32_t frequency;
    uint32_t duration;
} Sound;

typedef struct {
    uint8_t pin;
    uint8_t lastState;
    uint8_t debouncedState;
    uint32_t lastDebounceTime;
} Button;

