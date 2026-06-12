/*

## Complete Layer-Shifting Code with Double-Click Reset

Here is the updated code implementing a high-accuracy, non-blocking double-click reset handler.
When you double-click any of the 8 encoder shafts, it detects the rapid double-tap, clears the stored value of that specific dial back to 0, and briefly flashes the LED White to give you tactile, visual confirmation.

## Technical Design Elements

* Non-Blocking Delays: Instead of calling standard sleep_ms() to handle the button bounce or confirmation flash (which would completely freeze up your controls), this code saves timestamps using to_ms_since_boot(). This lets the Pico read encoder movement seamlessly even during a button tap.
* Edge-Trigger Filtering: Press actions are caught right when the button transitions from open to closed (is_pressed && !was_pressed), ensuring a prolonged hold doesn't accidentally trigger a double-click reset command.

*/

#include <stdio.h>#include <string.h>#include "pico/stdlib.h"#include "hardware/i2c.h"
// U153 Registers
#define U153_I2C_ADDR       0x41
#define REG_ENCODER_START   0x00
#define REG_LED_START       0x20
#define REG_BUTTON_BYTE     0x70
#define REG_SWITCH_BYTE     0x80
#define I2C_PORT            i2c0
#define PIN_SDA             4
#define PIN_SCL             5

// Timing configurations for click detection (in milliseconds)
#define DEBOUNCE_MS         30
#define DOUBLE_CLICK_WINDOW 300
#define FLASH_DURATION_MS   150

// Helper functions for I2C communication

bool u153_read_registers(uint8_t reg_addr, uint8_t *buffer, size_t length) {
    int bytes_written = i2c_write_blocking(I2C_PORT, U153_I2C_ADDR, &reg_addr, 1, true);
    if (bytes_written < 0) return false;
    int bytes_read = i2c_read_blocking(I2C_PORT, U153_I2C_ADDR, buffer, length, false);
    return (bytes_read == (int)length);
}

bool u153_update_all_leds(uint8_t *led_rgb_array) {
    uint8_t tx_buffer[25];
    tx_buffer[0] = REG_LED_START;
    memcpy(&tx_buffer[1], led_rgb_array, 24);
    int bytes_written = i2c_write_blocking(I2C_PORT, U153_I2C_ADDR, tx_buffer, sizeof(tx_buffer), false);
    return (bytes_written == sizeof(tx_buffer));
}

int main() {
    stdio_init_all();
    
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);

    sleep_ms(2000);
    printf("U153 Layer-Shifting Driver with Double-Click Reset Initialised.\n");

    // Hardware polling buffers
    uint8_t enc_raw_data[32];
    int32_t hardware_encoders[8] = {0};
    int32_t last_hardware_encoders[8] = {0};
    uint8_t button_byte = 0xFF;
    uint8_t last_button_byte = 0xFF;
    uint8_t switch_state = 0;
    uint8_t last_switch_state = 0xFF;

    // Software operational arrays
    int32_t layer_a_values[8] = {0};
    int32_t layer_b_values[8] = {0};
    uint8_t led_states[24] = {0}; 

    // Click tracking structures for all 8 buttons
    uint32_t last_press_time[8] = {0};
    uint32_t last_release_time[8] = {0};
    uint32_t flash_end_time[8] = {0};

    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());

        bool enc_success = u153_read_registers(REG_ENCODER_START, enc_raw_data, 32);
        bool btn_success = u153_read_registers(REG_BUTTON_BYTE, &button_byte, 1);
        bool sw_success  = u153_read_registers(REG_SWITCH_BYTE, &switch_state, 1);

        if (enc_success && btn_success && sw_success) {
            bool print_update = false;

            // Handle Global Layer Switch Changed
            if (switch_state != last_switch_state) {
                printf("\n--- Layer Changed to: %s ---\n", (switch_state == 0) ? "LAYER A (Green)" : "LAYER B (Red)");
                last_switch_state = switch_state;
                print_update = true;
            }

            // Core loop for each of the 8 encoder channels
            for (int i = 0; i < 8; i++) {
                // 1. EXTRACT ENCODER REVOLUTION DELTAS
                int base_idx = i * 4;
                hardware_encoders[i] = (int32_t)(
                    enc_raw_data[base_idx]         |
                    (enc_raw_data[base_idx + 1] << 8)  |
                    (enc_raw_data[base_idx + 2] << 16) |
                    (enc_raw_data[base_idx + 3] << 24)
                );

                int32_t delta = hardware_encoders[i] - last_hardware_encoders[i];
                if (delta != 0) {
                    print_update = true;
                    last_hardware_encoders[i] = hardware_encoders[i];
                    
                    if (switch_state == 0) {
                        layer_a_values[i] += delta;
                        if (layer_a_values[i] < 0) layer_a_values[i] = 0;
                        if (layer_a_values[i] > 100) layer_a_values[i] = 100;
                    } else {
                        layer_b_values[i] += delta;
                        if (layer_b_values[i] < 0) layer_b_values[i] = 0;
                        if (layer_b_values[i] > 100) layer_b_values[i] = 100;
                    }
                }

                // 2. STATE MACHINE: DOUBLE CLICK RESET DETECTION
                bool is_pressed = !(button_byte & (1 << i));
                bool was_pressed = !(last_button_byte & (1 << i));

                if (is_pressed && !was_pressed) { // Edge Trigger: Button Just Pressed
                    if ((now - last_press_time[i]) > DEBOUNCE_MS) {
                        // Check if the time since the PREVIOUS click falls within double click window
                        if ((now - last_press_time[i]) < DOUBLE_CLICK_WINDOW) {
                            // Double click verified! Reset values on active layer
                            if (switch_state == 0) layer_a_values[i] = 0;
                            else layer_b_values[i] = 0;
                            
                            flash_end_time[i] = now + FLASH_DURATION_MS; // Queue a quick flash visual
                            print_update = true;
                        }
                        last_press_time[i] = now;
                    }
                }

                // 3. LED RENDERING MATRIX
                if (now < flash_end_time[i]) {
                    // Flash White confirmation on reset
                    led_states[i * 3]     = 60; // Red
                    led_states[i * 3 + 1] = 60; // Green
                    led_states[i * 3 + 2] = 60; // Blue
                } else if (is_pressed) {
                    // Standard single hold gives a Blue feedback line
                    led_states[i * 3]     = 0;   
                    led_states[i * 3 + 1] = 0;   
                    led_states[i * 3 + 2] = 60;  
                } else {
                    // Normal operational lighting (Layer A = Green, Layer B = Red)
                    if (switch_state == 0) {
                        led_states[i * 3]     = 0;
                        led_states[i * 3 + 1] = layer_a_values[i];
                        led_states[i * 3 + 2] = 0;
                    } else {
                        led_states[i * 3]     = layer_b_values[i];
                        led_states[i * 3 + 1] = 0;
                        led_states[i * 3 + 2] = 0;
                    }
                }
            }

            last_button_byte = button_byte;
            u153_update_all_leds(led_states);

            if (print_update) {
                if (switch_state == 0) {
                    printf("Active Layer A (Green): ");
                    for(int i=0; i<8; i++) printf("[%d] ", layer_a_values[i]);
                } else {
                    printf("Active Layer B (Red):   ");
                    for(int i=0; i<8; i++) printf("[%d] ", layer_b_values[i]);
                }
                printf("\n");
            }
        }

        sleep_ms(20); // Polling step frequency optimized for fast click intervals
    }
}