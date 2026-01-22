// Depth-reactive effect: lights pressed keys with brightness based on depth
RGB_MATRIX_EFFECT(DEPTH_REACTIVE)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include "analog_matrix.h"

bool DEPTH_REACTIVE(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    // Clear all LEDs on init
    if (params->init) {
        rgb_matrix_set_color_all(0, 0, 0);
    }

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t led_index = g_led_config.matrix_co[row][col];

            // Skip if no LED or outside current processing range
            if (led_index == NO_LED) continue;
            if (led_index < led_min || led_index >= led_max) continue;

            uint8_t travel = analog_matrix_get_travel(row, col);

            // Threshold to avoid noise from resting keys
            if (travel > 5) {
                // Scale travel (15-180) to brightness (0-255)
                uint16_t linear = ((uint16_t)(travel - 5) * 255) / 180;
                uint8_t brightness = 0;

                if (linear > 255) {
                    brightness = 255;
                } else {
                    brightness = (uint8_t)linear;
                }

                // Use configured HSV color with depth-based brightness
                HSV hsv = rgb_matrix_config.hsv;
                hsv.v = scale8(brightness, hsv.v);  // Scale to max configured brightness
                RGB rgb = rgb_matrix_hsv_to_rgb(hsv);

                rgb_matrix_set_color(led_index, rgb.r, rgb.g, rgb.b);
            } else {
                // Key not pressed - turn off LED
                rgb_matrix_set_color(led_index, 0, 0, 0);
            }
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
