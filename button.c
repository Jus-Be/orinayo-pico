/*
 * button.c
 *
 * Handles physical button state (BOOTSEL on Pico) and generates logical button events.
 * Internally uses a state machine to detect short press, long press, and release.
 *
 *
 * Copyright 2025, Hiroyuki OYAMA
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "hardware/gpio.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

#include "button.h"

#define BUTTON_DEBOUNCE_COUNT 5                    // consecutive reads needed for stable state
#define PRESS_DURATION_US (500 * 1000)             // 500 ms
#define LONG_PRESS_DURATION_US (2000 * 1000)       // 2 s
#define VERY_LONG_PRESS_DURATION_US (5000 * 1000)  // 5 s

typedef enum {
    BUTTON_STATE_IDLE = 0,
    BUTTON_STATE_PRESS_DOWN,
    BUTTON_STATE_HOLD_ACTIVE,
    BUTTON_STATE_LONG_HOLD_ACTIVE,
    BUTTON_STATE_VERY_LONG_HOLD_ACTIVE,
} button_state_t;

typedef struct {
    button_state_t state;
    uint64_t press_start_us;
} button_fsm_t;

extern bool button_current_down;


/*
 * Reads BOOTSEL button state and returns a button_event_t (see button.h).
 * Maintains internal FSM to distinguish short press, long press, and release.
 */
button_event_t button_poll_event(void) {
    static button_fsm_t fsm = {0};
    button_event_t ev = BUTTON_EVENT_NONE;
    uint64_t now_us = time_us_64();

    switch (fsm.state) {
        case BUTTON_STATE_IDLE:
            if (button_current_down) {
                fsm.press_start_us = now_us;
                fsm.state = BUTTON_STATE_PRESS_DOWN;
                ev = BUTTON_EVENT_DOWN;
            }
            break;
        case BUTTON_STATE_PRESS_DOWN:
            if (!button_current_down) {
                fsm.state = BUTTON_STATE_IDLE;
                ev = BUTTON_EVENT_CLICK_RELEASE;
            } else if (now_us - fsm.press_start_us > PRESS_DURATION_US) {
                fsm.state = BUTTON_STATE_HOLD_ACTIVE;
                ev = BUTTON_EVENT_HOLD_BEGIN;
            }
            break;
        case BUTTON_STATE_HOLD_ACTIVE:
            if (!button_current_down) {
                fsm.state = BUTTON_STATE_IDLE;
                ev = BUTTON_EVENT_HOLD_RELEASE;
            } else if (now_us - fsm.press_start_us > LONG_PRESS_DURATION_US) {
                fsm.state = BUTTON_STATE_LONG_HOLD_ACTIVE;
                ev = BUTTON_EVENT_LONG_HOLD_BEGIN;
            }
            break;
        case BUTTON_STATE_LONG_HOLD_ACTIVE:
            if (!button_current_down) {
                fsm.state = BUTTON_STATE_IDLE;
                ev = BUTTON_EVENT_LONG_HOLD_RELEASE;
            } else if (now_us - fsm.press_start_us > VERY_LONG_PRESS_DURATION_US) {
                fsm.state = BUTTON_STATE_VERY_LONG_HOLD_ACTIVE;
                ev = BUTTON_EVENT_VERY_LONG_HOLD_BEGIN;
            }
            break;
        case BUTTON_STATE_VERY_LONG_HOLD_ACTIVE:
            if (!button_current_down) {
                fsm.state = BUTTON_STATE_IDLE;
                ev = BUTTON_EVENT_VERY_LONG_HOLD_RELEASE;
            }
            break;
    }
    return ev;
}
