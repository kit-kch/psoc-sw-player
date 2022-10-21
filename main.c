// #################################################################################################
// # << NEORV32 - Blinking LED Demo Program >>                                                     #
// # ********************************************************************************************* #
// # BSD 3-Clause License                                                                          #
// #                                                                                               #
// # Copyright (c) 2021, Stephan Nolting. All rights reserved.                                     #
// #                                                                                               #
// # Redistribution and use in source and binary forms, with or without modification, are          #
// # permitted provided that the following conditions are met:                                     #
// #                                                                                               #
// # 1. Redistributions of source code must retain the above copyright notice, this list of        #
// #    conditions and the following disclaimer.                                                   #
// #                                                                                               #
// # 2. Redistributions in binary form must reproduce the above copyright notice, this list of     #
// #    conditions and the following disclaimer in the documentation and/or other materials        #
// #    provided with the distribution.                                                            #
// #                                                                                               #
// # 3. Neither the name of the copyright holder nor the names of its contributors may be used to  #
// #    endorse or promote products derived from this software without specific prior written      #
// #    permission.                                                                                #
// #                                                                                               #
// # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   #
// # OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               #
// # MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    #
// # COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     #
// # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE #
// # GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    #
// # AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     #
// # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  #
// # OF THE POSSIBILITY OF SUCH DAMAGE.                                                            #
// # ********************************************************************************************* #
// # The NEORV32 Processor - https://github.com/stnolting/neorv32              (c) Stephan Nolting #
// #################################################################################################


/**********************************************************************//**
 * @file blink_led/main.c
 * @author Stephan Nolting
 * @brief Simple blinking LED demo program using the lowest 8 bits of the GPIO.output port.
 **************************************************************************/

#include <neorv32.h>
#include <stdbool.h>
#include "bsp_psoc_board.h"


/**********************************************************************//**
 * @name User configuration
 **************************************************************************/
/**@{*/
/** UART BAUD rate */
#define BAUD_RATE 19200
/**@}*/

// Global variables
uint32_t spi_configured;
uint32_t spi_size; // data quantity in bytes

/** SPI Prescalers */
enum SPI_PRESCALER_ENUM{
    SPI_PRESC_2    = 0,
    SPI_PRESC_4    = 1,
    SPI_PRESC_8    = 2,
    SPI_PRESC_64   = 3,
    SPI_PRESC_128  = 4,
    SPI_PRESC_1024 = 5,
    SPI_PRESC_2048 = 6,
    SPI_PRESC_4096 = 7,
};

// Prototypes
void sd_card_test(void);
void oled_display_test(void);
bool check_button_clicked(int pin);
void spi_setup(enum SPI_PRESCALER_ENUM prsc, uint8_t clock_mode, uint8_t rxtx_size);
uint32_t hexstr_to_uint(char *buffer, uint8_t length);
void aux_print_hex_byte(uint8_t byte);


/**********************************************************************//**
 * SPI flash commands
 **************************************************************************/
enum SPI_FLASH_CMD {
  SPI_FLASH_CMD_WRITE    = 0x02, /**< Write data */
  SPI_FLASH_CMD_READ     = 0x03, /**< Read data */
  SPI_FLASH_CMD_READ_SR  = 0x05, /**< Get status register */
  SPI_FLASH_CMD_WREN     = 0x06  /**< Enable write access */
};


/**********************************************************************//**
 * Tests peripherals of the PSoC board
 *
 * @note This program requires the GPIO, PWM, TWI and SPI controller to be synthesized (the UART is optional).
 *
 * @return 0 if execution was successful
 **************************************************************************/
int main() {

  // init UART (primary UART = UART0; if no id number is specified the primary UART is used) at default baud rate, no parity bits, ho hw flow control
  neorv32_uart0_setup(BAUD_RATE, PARITY_NONE, FLOW_CONTROL_NONE);

  // check if GPIO unit is implemented at all
  if (neorv32_gpio_available() == 0) {
    neorv32_uart0_print("Error! No GPIO unit synthesized!\n");
    return 1; // nope, no GPIO unit synthesized
  }

  // check if TWI unit is implemented at all
  if (neorv32_twi_available() == 0) {
    neorv32_uart0_printf("No TWI unit implemented.");
    return 1;
  }

  // check if SPI unit is implemented at all
  if (neorv32_spi_available() == 0) {
    neorv32_uart0_printf("No SPI unit implemented.");
    return 1;
  }

  // check if SPI unit is implemented at all
  if (neorv32_pwm_available() == 0) {
    neorv32_uart0_printf("No PWM unit implemented.");
    return 1;
  }


  // capture all exceptions and give debug info via UART
  // this is not required, but keeps us safe
  neorv32_rte_setup();

  // say hello
  neorv32_uart0_print("PSoC Board demo program\n");

  /** Init PWM */
  neorv32_pwm_setup(CLK_PRSC_1024);
  neorv32_pwm_enable();
  neorv32_pwm_set(PWM_LED_CH, 50);

  /* Init SPI */
  spi_setup(SPI_PRESC_2, 0, 1);

  /* Init TWI */
  neorv32_twi_setup(CLK_PRSC_2048);
  neorv32_twi_enable();

  /** I2C Display test: read I2C address of the display and maybe some ID
   * and then check the ID
   */
  oled_display_test();

  /** SD Card test */
  sd_card_test();

  /** GPIO and LED tests:
   * - Output log message when button is pressed
   * - Switch on and off LEDs
   */
  neorv32_gpio_pin_set(LED_D2);
  uint8_t led_val = 0;
  while(1) {
      // Debounce delay
      neorv32_cpu_delay_ms(100);

      neorv32_pwm_set(PWM_LED_CH, led_val++);

      // Buttons check
      if(check_button_clicked(BUTTON_UP)) {neorv32_uart0_print("UP button clicked\n"); }
      if(check_button_clicked(BUTTON_DOWN)) {neorv32_uart0_print("DOWN button clicked\n"); }
      if(check_button_clicked(BUTTON_LEFT)) {neorv32_uart0_print("LEFT button clicked\n"); }
      if(check_button_clicked(BUTTON_RIGHT)) {neorv32_uart0_print("RIGHT button clicked\n"); }
      if(check_button_clicked(BUTTON_CENTER)) {neorv32_uart0_print("CENTER button clicked\n"); }
      if(check_button_clicked(BUTTON_SW2)) {neorv32_uart0_print("SW2 button clicked\n"); }
      if(check_button_clicked(BUTTON_SW3)) {neorv32_uart0_print("SW3 button clicked\n"); }      

      // Switch LEDs
      neorv32_gpio_pin_toggle(LED_D2);
      neorv32_gpio_pin_toggle(LED_D3);

      // Test ASIC audio lines
      neorv32_gpio_pin_toggle(ASIC_AUDIO_LEFT);
      neorv32_gpio_pin_toggle(ASIC_AUDIO_RIGHT);
  }
  return 0;
}

/**
 * Checks button click condition 
 */
bool check_button_clicked(int pin){
    static bool pressed;

    if(pressed){
        if(!neorv32_gpio_pin_get(pin)){
            pressed = false;
            return true;
        }
    } else {
        if(neorv32_gpio_pin_get(pin))
            pressed = true;
    }
    return false;
}

/**
 * Setup SPI module
 * 
 * @arg prsc - prescaler, value from SPI_PRESCALER_ENUM enumeration
 * @arg clock_mode - SPI clock mode 0x00 .. 0x03 (CPHA and CPOL bits)
 * @arg rxtx_size - SPI transfer size: (1,2,3,4)
 */
void spi_setup(enum SPI_PRESCALER_ENUM prsc, uint8_t clock_mode, uint8_t rxtx_size) {

  uint8_t clk_phase, clk_pol, data_size;

  // ---- SPI clock ----
  uint32_t div = 0;
  switch (prsc) {
    case SPI_PRESC_2: div = 2 * 2; break;
    case SPI_PRESC_4: div = 2 * 4; break;
    case SPI_PRESC_8: div = 2 * 8; break;
    case SPI_PRESC_64: div = 2 * 64; break;
    case SPI_PRESC_128: div = 2 * 128; break;
    case SPI_PRESC_1024: div = 2 * 1024; break;
    case SPI_PRESC_2048: div = 2 * 2048; break;
    case SPI_PRESC_4096: div = 2 * 4096; break;
    default: div = 0; break;
  }
  uint32_t clock = NEORV32_SYSINFO.CLK / div;
  neorv32_uart0_printf("\n+ SPI clock speed = %u Hz\n", clock);

  // ---- SPI clock mode ----
  clk_pol   = (uint8_t)((clock_mode >> 1) & 0x01);
  clk_phase = (uint8_t)(clock_mode & 0x01);
  neorv32_uart0_printf("\n+ SPI clock mode = %u\n", clock_mode);

  // ---- SPI transfer data quantity ----
  data_size = rxtx_size - 1;
  neorv32_uart0_printf("\n+ New SPI data size = %u-byte(s)\n\n", rxtx_size);

  neorv32_spi_setup(prsc, clk_phase, clk_pol, data_size);
  spi_configured = 1; // SPI is configured now
  spi_size = rxtx_size;
}

/**
 * Set SD-Card in SPI mode and check response according to:
 * https://www.st.com/resource/en/application_note/an5595-spc58xexspc58xgx-multimedia-card-via-spi-interface-stmicroelectronics.pdf
 */
void sd_card_test(void){
    uint8_t channel = SD_CS_CHANNEL;

    // SD must be deselected during this
    neorv32_uart0_printf("Syncing SD Card Clock...\n");
    for(uint8_t i = 0; i < 10; i++)
        neorv32_spi_trans(0xFF);

    neorv32_uart0_printf("Sending CMD0 to SD Card...\n");

    neorv32_spi_cs_en(channel);
    neorv32_spi_trans(0x40);
    neorv32_spi_trans(0x00);
    neorv32_spi_trans(0x00);
    neorv32_spi_trans(0x00);
    neorv32_spi_trans(0x00);
    neorv32_spi_trans(0x95);
    neorv32_spi_cs_dis(channel);
    neorv32_cpu_delay_ms(1);

    neorv32_spi_cs_en(channel);
    uint8_t resp = 0xFF;
    for (size_t i = 0; i < 8; i++)
    {
      resp = neorv32_spi_trans(0x00);
      if (resp != 0xFF)
        break;
    }
    neorv32_spi_cs_dis(channel);
    
    if(resp == 0x01){
        neorv32_uart0_printf("[OK] SD Card response 0x01 (IDLE_STATE).\n");
    } else {
        neorv32_uart0_printf("[ERROR] SD Card response %u: wrong response.\n", resp);
    }
}

/**
 * Write two commands: set display on (0xAF) and then set entire display on (0xA5)
 * @ref https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
 */
void oled_display_test(void){
    int nack = 0;
    /** Switch display on (0xAF) */
    neorv32_twi_generate_start();
    // Start write
    nack = neorv32_twi_trans((OLED_DISP_ADDRESS<<1)|0x00);
    if(nack) {
        neorv32_uart0_printf("[ERROR] OLED Display NACK on starting write transaction.\n");
        return;
    }
    // Next byte is control byte
    nack = neorv32_twi_trans(0x80);
    if(nack) {
        neorv32_uart0_printf("[ERROR] OLED Display NACK on command byte marker (0x80).\n");
        return;
    }
    // Switch display ON
    nack = neorv32_twi_trans(0xAF);
    if(nack) {
        neorv32_uart0_printf("[ERROR] OLED Display NACK on switch display on command (0xAF).\n");
        return;
    }
    neorv32_twi_generate_stop();

    neorv32_cpu_delay_ms(1);

    /** Switch entire display on (0xA5) */
    neorv32_twi_generate_start();
    // Start write
    nack = neorv32_twi_trans((OLED_DISP_ADDRESS<<1)|0x00);
    if(nack) {
        neorv32_uart0_printf("[ERROR] OLED Display NACK on starting write transaction.\n");
        return;
    }
    // Next byte is control byte
    nack = neorv32_twi_trans(0x80);
    if(nack) {
        neorv32_uart0_printf("[ERROR] OLED Display NACK on command byte marker (0x80).\n");
        return;
    }
    // Switch display ON
    nack = neorv32_twi_trans(0xA5);
    if(nack) {
        neorv32_uart0_printf("[ERROR] OLED Display NACK on switch entire display on command (0xA5).\n");
        return;
    }
    neorv32_twi_generate_stop();
}

/**********************************************************************//**
 * Helper function to convert N hex chars string into uint32_T
 *
 * @param[in,out] buffer Pointer to array of chars to convert into number.
 * @param[in,out] length Length of the conversion string.
 * @return Converted number.
 **************************************************************************/
uint32_t hexstr_to_uint(char *buffer, uint8_t length) {

  uint32_t res = 0, d = 0;
  char c = 0;

  while (length--) {
    c = *buffer++;

    if ((c >= '0') && (c <= '9'))
      d = (uint32_t)(c - '0');
    else if ((c >= 'a') && (c <= 'f'))
      d = (uint32_t)((c - 'a') + 10);
    else if ((c >= 'A') && (c <= 'F'))
      d = (uint32_t)((c - 'A') + 10);
    else
      d = 0;

    res = res + (d << (length*4));
  }

  return res;
}


/**********************************************************************//**
 * Print HEX byte.
 *
 * @param[in] byte Byte to be printed as 2-cahr hex value.
 **************************************************************************/
void aux_print_hex_byte(uint8_t byte) {

  static const char symbols[] = "0123456789abcdef";

  neorv32_uart0_putc(symbols[(byte >> 4) & 0x0f]);
  neorv32_uart0_putc(symbols[(byte >> 0) & 0x0f]);
}
