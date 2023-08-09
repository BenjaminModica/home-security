#ifndef PICO_TCP_MULTICORE_SERVER_H
#define PICO_TCP_MULTICORE_SERVER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "boards/pico_w.h"

#include "../libs/easytcp.h"
#include "../libs/mfrc522.h"

#define LED_PIN_GREEN 28
#define LED_PIN_RED 27
#define LED_PIN_YELLOW 22
#define PIEZO_PIN 14
#define PIR_PIN 26
#define BTN_PIN 15

#define DEACTIVATED_STATE 0x31 //1
#define ACTIVATED_STATE 0x32 //2
#define TRIPPED_STATE 0x33 //3
#define ALARM_STATE 0x34 //4

#define ALARM_DEACTIVATION 0x30 //0
#define ALARM_ACTIVATION 0x31 //1

static uint8_t app_state;
static uint8_t auth_counter;
static bool timed_out;

struct repeating_timer timer_slow;
struct repeating_timer timer_fast;
struct repeating_timer timer_counter;

bool repeating_timer_callback(struct repeating_timer *t);
bool repeating_timer_callback_counter(struct repeating_timer *t);

#endif