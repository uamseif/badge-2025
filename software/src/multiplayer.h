#include <string.h>

#include "lib_uart.h"

#define MAX_MSG_LEN 32  // Max message size (adjust as needed)

#define MSG_ACTION_PING 0x01
#define MSG_ACTION_ACK 0x02
#define MSG_ACTION_START_GAME 0x10
#define MSG_ACTION_ACCEPT_GAME 0x11
#define MSG_ACTION_GAME_MESSAGE 0x20



static uint8_t message_buffer[MAX_MSG_LEN];
static uint8_t message_index = 0;
static uint8_t expected_length = 0;
const uint8_t *multiplayer_game_data = NULL;

extern uint8_t multiplayer_accepted_game;


static inline void accept_game(const uint8_t game_id)
{
    uint8_t message[4];
    message[0] = MSG_ACTION_ACCEPT_GAME;
    message[1] = 0x01;
    message[2] = game_id;
    message[3] = message[0] ^ message[1] ^ message[2];  // CRC

    uart_write(message, sizeof(message));
}

static inline void start_multiplayer_game(const uint8_t game_id)
{
    uint8_t message[4];
    message[0] = MSG_ACTION_START_GAME;
    message[1] = 0x01;
    message[2] = game_id;
    message[3] = message[0] ^ message[1] ^ message[2];  // CRC

    uart_write(message, sizeof(message));
}

static inline void send_game_message(const uint8_t game_id, const uint8_t *game_message, const uint8_t message_length) {

    // Total size of the message (header + game message + CRC)
    uint8_t total_message_size = 4 + message_length;

    // Create a new buffer to hold the complete message
    uint8_t message[total_message_size];

    // Message Header
    message[0] = MSG_ACTION_GAME_MESSAGE;    // Action ID (specific to the game message)
    message[1] = message_length + 1;              // Length of the game message
    message[2] = game_id;                    // Game ID
    memcpy(&message[3], game_message, message_length);  // Copy game message content

    // Calculate CRC (XOR all the header + game message bytes)
    uint8_t crc = 0;
    for (uint8_t i = 0; i < total_message_size - 1; i++) {
        crc ^= message[i];
    }
    message[total_message_size - 1] = crc;  // CRC at the end

    // Send the complete message over UART
    uart_write(message, total_message_size);
}

static inline void process_uart()
{
    uint8_t byte_read;
    while(uart_read(&byte_read, 1) == 1)  // Read one byte at a time if available
    {
        // Accumulate bytes into message buffer
        if(message_index < MAX_MSG_LEN)
        {
            message_buffer[message_index++] = byte_read;

            if(message_index == 2)
            {
                // Received Action ID and Length, calculate expected total length
                expected_length = 2 + message_buffer[1] + 1;  // ID + Length + Data + CRC
            }

            if(message_index == expected_length && expected_length > 0)
            {
                // Full message received
                uint8_t calculated_crc = 0;
                for(uint8_t i = 0; i < expected_length - 1; i++)
                {
                    calculated_crc ^= message_buffer[i];
                }

                uint8_t received_crc = message_buffer[expected_length - 1];

                if(calculated_crc == received_crc)
                {
                    // CRC is valid -> process the message
                    const uint8_t action_id = message_buffer[0];
                    const uint8_t data_length = message_buffer[1];
                    const uint8_t *data = &message_buffer[2];

                    switch(action_id)
                    {
                        case MSG_ACTION_START_GAME:
                            if(data_length == 1)
                            {
                                const uint8_t game_id = data[0];
#ifdef DEBUG
                                printf("game_id: %02x\n", game_id);
#endif

                                accept_game(game_id);
                                multiplayer_accepted_game = game_id;
                            }
                            break;
                        case MSG_ACTION_ACCEPT_GAME:
                            if(data_length == 1)
                            {
                                //const  uint8_t game_id = data[0];
#ifdef DEBUG
                                printf("game_id: %02x\n", game_id);
#endif
                            }
                            break;

                        case MSG_ACTION_GAME_MESSAGE:
                            if(data_length > 1)
                            {
                                //const  uint8_t game_id = data[0];
#ifdef DEBUG
                                printf("game_message: game_id: %02x\n", game_id);
#endif
                                multiplayer_game_data = data;
                            }
                        break;


                        default:
                            break;
                    }
                }
                else
                {
                    // CRC mismatch: discard message
#ifdef DEBUG
                    printf("Unreceived CRC: 0x%02X\n", calculated_crc);
#endif
                }

                // Reset buffer for next message
                message_index = 0;
                expected_length = 0;
            }
        }
        else
        {
            message_index = 0;
            expected_length = 0;
#ifdef DEBUG
            printf("Unexpected message length: %d\n", message_index);
#endif
        }
    }
}