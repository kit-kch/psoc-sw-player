/** Board support package for the PSoC Board */
#pragma once

/** SD Card */
#define SD_CS_CHANNEL (1)

/** OLED Display */
#define OLED_DISP_ADDRESS (0x3C) // Or (0x3D), << 1

/** LEDs */
#define PWM_LED_CH (0)
#define LED_D2 (0) 
#define LED_D3 (1)

/** Buttons */
#define BUTTON_UP     (3)
#define BUTTON_DOWN   (5)
#define BUTTON_LEFT   (4)
#define BUTTON_RIGHT  (2)
#define BUTTON_CENTER (6)

#define BUTTON_SW2 (0)
#define BUTTON_SW3 (1)

/** ASIC Audio pins */
#define ASIC_AUDIO_LEFT (2)
#define ASIC_AUDIO_RIGHT (3)
