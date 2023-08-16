/**
 * Author: Benjamin Modica 2023
 */

#include "main.h"

/**
 * This is the "main" function for the second core running wifi stuff and TCP server
*/
void wifi_core() {
    TCP_SERVER_T *easytcp_state = easytcp_init();

    while(1) {
        sleep_ms(1000);
        //Receive all data, send the latest value to main core.
        uint8_t recv_data[RINGBUF_SIZE];
        int nbr_of_data = easytcp_receive_data(easytcp_state, recv_data);
        if (nbr_of_data) {
            multicore_fifo_push_blocking(recv_data[nbr_of_data - 1]);
        }

        //Send status of alarm (spamming much??)
        uint8_t send_data;
        switch(app_state) {
            case DEACTIVATED_STATE:
                send_data = 0x31; //1 in ascii
                easytcp_send_data(easytcp_state, send_data);
                break;
            case ACTIVATED_STATE:
                send_data = 0x32; //2 in ascii
                easytcp_send_data(easytcp_state, send_data);
                break;
            case TRIPPED_STATE:
                send_data = 0x33; //3 in ascii
                easytcp_send_data(easytcp_state, send_data);
                break;
            case ALARM_STATE:
                send_data = 0x34; //4 in ascii
                easytcp_send_data(easytcp_state, send_data);
                break;
            default:
                send_data = 0x45; //E in ascii (for error)
                easytcp_send_data(easytcp_state, send_data);
        }
    }
}

int main() {
    stdio_init_all();

    //Declare owned tags
    uint8_t tag1[] = {0x93, 0xE3, 0x9A, 0x92};
    uint8_t tag2[] = {0x03, 0x3A, 0xB8, 0x91};
    app_state = DEACTIVATED_STATE;
    auth_counter = 0;
    timed_out = false;

    MFRC522Ptr_t mfrc = MFRC522_Init();
    PCD_Init(mfrc, spi0);

    multicore_launch_core1(wifi_core);

    gpio_init(LED_PIN_GREEN);
    gpio_init(LED_PIN_RED);
    gpio_init(LED_PIN_YELLOW);
    gpio_init(PIEZO_PIN);
    gpio_init(PIR_PIN);
    gpio_init(BTN_PIN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    gpio_set_dir(LED_PIN_YELLOW, GPIO_OUT);
    gpio_set_dir(PIEZO_PIN, GPIO_OUT);
    gpio_set_dir(PIR_PIN, GPIO_IN);
    gpio_set_dir(BTN_PIN, GPIO_IN);

    while(1) {
        uint8_t command;

        switch (app_state) {
            case DEACTIVATED_STATE:
                //Fix so that wifi exits this state
                gpio_put(LED_PIN_GREEN, 1);

                if (multicore_fifo_rvalid()) {
                    command = multicore_fifo_pop_blocking();
                    if (command == ALARM_ACTIVATION) app_state = ACTIVATED_STATE;
                }
                
                //For button activation (in the future add timer here?)
                if (gpio_get(BTN_PIN) == 1) app_state = ACTIVATED_STATE;
                break;
            case ACTIVATED_STATE:
                //Wait for PIR sensor to trip
                gpio_put(LED_PIN_GREEN, 0);

                if (multicore_fifo_rvalid()) {
                    command = multicore_fifo_pop_blocking();
                    if (command == ALARM_DEACTIVATION) app_state = DEACTIVATED_STATE;
                }

                if (gpio_get(PIR_PIN) == 1) {
                    app_state = TRIPPED_STATE;
                }
                break;
            case TRIPPED_STATE:
                auth_counter = 0;
                gpio_put(LED_PIN_YELLOW, 1);
                add_repeating_timer_ms(500, repeating_timer_callback, NULL, &timer_slow);
                add_repeating_timer_ms(1000, repeating_timer_callback_counter, NULL, &timer_counter);
                timed_out = false;
                //Clear UID

                //Wait for new card
                printf("Waiting for card\n\r");
                while(auth_counter < 10 && !PICC_IsNewCardPresent(mfrc));
                if(auth_counter >= 10) timed_out = true;
                //Select the card
                printf("Selecting card\n\r");
                PICC_ReadCardSerial(mfrc);

                //Show UID on serial monitor
                printf("PICC dump: \n\r");
                PICC_DumpToSerial(mfrc, &(mfrc->uid));

                //Authorization with uid
                printf("Uid is: ");
                for(int i = 0; i < 4; i++) {
                    printf("%x ", mfrc->uid.uidByte[i]);
                } printf("\n\r");

                if(( memcmp(mfrc->uid.uidByte, tag1, 4) == 0 || memcmp(mfrc->uid.uidByte, tag2, 4) == 0 ) && !timed_out) {
                    printf("Authentication Success\n\r");
                    gpio_put(LED_PIN_GREEN, 1);
                    gpio_put(LED_PIN_RED, 0);
                    gpio_put(LED_PIN_YELLOW, 0);
                    gpio_put(PIEZO_PIN, 0);
                    cancel_repeating_timer(&timer_slow);
                    cancel_repeating_timer(&timer_counter);
                    app_state = DEACTIVATED_STATE;
                } else {
                    printf("Authentication Failed\n\r");
                    gpio_put(LED_PIN_GREEN, 0);
                    gpio_put(LED_PIN_RED, 1);
                    gpio_put(LED_PIN_YELLOW, 0);
                    gpio_put(PIEZO_PIN, 1);
                    cancel_repeating_timer(&timer_slow);
                    cancel_repeating_timer(&timer_counter);
                    app_state = ALARM_STATE;
                }
                break;
            case ALARM_STATE:
                gpio_put(LED_PIN_RED, 1);
                add_repeating_timer_ms(100, repeating_timer_callback, NULL, &timer_fast);
                sleep_ms(5000);
                app_state = DEACTIVATED_STATE;
                cancel_repeating_timer(&timer_fast);
                gpio_put(LED_PIN_RED, 0);
                break;
            default:
                printf("Error unknown state");
        }  
    }
    return 0;
}

bool repeating_timer_callback_counter(struct repeating_timer *t) {
    auth_counter++;
}

bool repeating_timer_callback(struct repeating_timer *t) {
    if (gpio_get(PIEZO_PIN) == 0) {
        gpio_put(PIEZO_PIN, 1);
    } else {
        gpio_put(PIEZO_PIN, 0);
    }
}

