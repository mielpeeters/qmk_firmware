// Depth-reactive effect: lights pressed keys with brightness based on depth
RGB_MATRIX_EFFECT(DEPTH_REACTIVE)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

#include "analog_matrix.h"

// Helper to get brightness from travel value
static uint8_t travel_to_brightness(uint8_t travel) {
    if (travel <= 5) return 0;
    uint16_t linear = ((uint16_t)(travel - 5) * 255) / 180;
    return linear > 255 ? 255 : (uint8_t)linear;
}

// Neighbor glow intensity (0-255, where 64 = 25% of pressed key brightness)
#define NEIGHBOR_GLOW_FACTOR 127

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

            // Get this key's own brightness
            uint8_t travel = analog_matrix_get_travel(row, col);
            uint16_t brightness = travel_to_brightness(travel);

            // Check neighboring keys and add their glow contribution
            for (int8_t dr = -1; dr <= 1; dr++) {
                for (int8_t dc = -1; dc <= 1; dc++) {
                    if (dr == 0 && dc == 0) continue;  // Skip self

                    int8_t nr = row + dr;
                    int8_t nc = col + dc;

                    // Bounds check
                    if (nr < 0 || nr >= MATRIX_ROWS) continue;
                    if (nc < 0 || nc >= MATRIX_COLS) continue;

                    uint8_t neighbor_travel = analog_matrix_get_travel(nr, nc);
                    uint8_t neighbor_brightness = travel_to_brightness(neighbor_travel);

                    // Add scaled neighbor contribution
                    brightness += scale8(neighbor_brightness, NEIGHBOR_GLOW_FACTOR);
                }
            }

            // Clamp to 255
            if (brightness > 255) brightness = 255;

            if (brightness > 0) {
                // Use configured HSV color with computed brightness
                HSV hsv = rgb_matrix_config.hsv;
                hsv.v = scale8((uint8_t)brightness, hsv.v);
                RGB rgb = rgb_matrix_hsv_to_rgb(hsv);

                rgb_matrix_set_color(led_index, rgb.r, rgb.g, rgb.b);
            } else {
                // No brightness - turn off LED
                rgb_matrix_set_color(led_index, 0, 0, 0);
            }
        }
    }

    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
