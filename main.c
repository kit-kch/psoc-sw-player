
#include <neorv32.h>
#include "psoc_board.h"

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


static void test_rst_button()
{
    neorv32_uart0_printf("* SW2 Button Test:\n");
    neorv32_uart0_printf("  => Press SW2 now. It's working if the SoC resets\n");
}

static void test_button(char* name, int gpio)
{
    neorv32_uart0_printf("  => Press & hold %s\n", name);
    while (neorv32_gpio_pin_get(gpio)){}
    neorv32_uart0_printf("  => Release %s\n", name);
    while (!neorv32_gpio_pin_get(gpio)){}
}

static void test_buttons()
{
    neorv32_uart0_printf("* Button Test:\n");
    test_button("Joystick LEFT", PSOC_BTN_L);
    test_button("Joystick UP", PSOC_BTN_U);
    test_button("Joystick RIGHT", PSOC_BTN_R);
    test_button("Joystick DOWN", PSOC_BTN_D);
    test_button("Joystick CENTER", PSOC_BTN_C);
    test_button("SW1", PSOC_BTN_SW1);
    neorv32_uart0_printf("  => [OK]\n");
}

static void test_uart()
{
    neorv32_uart0_printf("* UART Test:\n");
    neorv32_uart0_printf("  => TX is working if you can read this\n");
    neorv32_uart0_printf("  => RX test: enter 1\n");
    char c = neorv32_uart0_getc();
    if (c != '1')
    {
        neorv32_uart0_printf("  => Wrong character '%c'\n", c);
        neorv32_uart0_printf("  => [FAIL]\n");
    }
    else
    {
        neorv32_uart0_printf("  => [OK]\n");
    }
}

static void spin_cpu_timer(uint32_t seconds)
{
    uint64_t timeout = neorv32_clint_time_get() + (uint64_t)(seconds * NEORV32_SYSINFO->CLK);
    while (neorv32_clint_time_get() < timeout)
    {
    }
}

static void test_led()
{
    neorv32_uart0_printf("* LED Test:\n");
    neorv32_uart0_printf("  => D2 on for five seconds\n");
    neorv32_gpio_pin_set(PSOC_LED2, 1);
    spin_cpu_timer(5);
    neorv32_uart0_printf("  => D2 off\n");
    neorv32_gpio_pin_set(PSOC_LED2, 0);
    neorv32_gpio_pin_set(PSOC_LED3, 1);
    neorv32_uart0_printf("  => D3 on for five seconds\n");
    spin_cpu_timer(5);
    neorv32_uart0_printf("  => D3 off\n");
    neorv32_gpio_pin_set(PSOC_LED3, 0);

    neorv32_uart0_printf("  => [DONE]\n");
}

static void test_sd_card()
{
    neorv32_uart0_printf("* SD Card Test:\n  => Not implemented\n  => [SKIPPED]\n");
}

// Or (0x3D), << 1
#define OLED_DISP_ADDRESS (0x3C)

static int oled_send_cmd(uint8_t cmd)
{
    neorv32_twi_generate_start();

    uint8_t data = (OLED_DISP_ADDRESS << 1);
    int nack = neorv32_twi_trans(&data, 0);
    if (nack)
        return 1;

    data = 0x80;
    nack = neorv32_twi_trans(&data, 0);
    if (nack)
        return 2;

    data = cmd;
    nack = neorv32_twi_trans(&data, 0);
    if (nack)
        return 3;
    neorv32_twi_generate_stop();

    return 0;
}

static void test_i2c_oled()
{
    /*
     * Write two commands: set display on (0xAF) and then set entire display on (0xA5)
     * https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
     */
    neorv32_uart0_printf("* I2C Test:\n");
    neorv32_twi_setup(CLK_PRSC_2048, 15, 0);
    int ret = oled_send_cmd(0xAF);
    if (ret != 0)
    {
        neorv32_uart0_printf("  => Got NACK in CMD 0xAF stage %d\n  => [FAIL]\n");
        return;
    }

    neorv32_cpu_delay_ms(1);
    ret = oled_send_cmd(0xA5);
    if (ret != 0)
    {
        neorv32_uart0_printf("  => Got NACK in CMD 0xA5 stage %d\n  => [FAIL]\n");
        return;
    }

    neorv32_uart0_printf("  => [OK]\n");
}

const size_t sin_buf_len = 90;
static int32_t sin_buf[90] = { 
    0x000000, 0x08edc7, 0x11d06c, 0x1a9cd9, 0x234815, 0x2bc750, 
    0x340ff1, 0x3c17a4, 0x43d464, 0x4b3c8b, 0x5246dc, 0x58ea90, 
    0x5f1f5d, 0x64dd88, 0x6a1de6, 0x6ed9ea, 0x730bae, 0x76adf4, 
    0x79bc37, 0x7c32a5, 0x7e0e2d, 0x7f4c7d, 0x7fec08, 0x7fec08, 
    0x7f4c7d, 0x7e0e2d, 0x7c32a5, 0x79bc37, 0x76adf4, 0x730bae, 
    0x6ed9ea, 0x6a1de6, 0x64dd88, 0x5f1f5d, 0x58ea90, 0x5246dc, 
    0x4b3c8b, 0x43d464, 0x3c17a4, 0x340ff1, 0x2bc750, 0x234815, 
    0x1a9cd9, 0x11d06c, 0x08edc7, 0x000000, 0xf71239, 0xee2f94, 
    0xe56327, 0xdcb7eb, 0xd438b0, 0xcbf00f, 0xc3e85c, 0xbc2b9c, 
    0xb4c375, 0xadb924, 0xa71570, 0xa0e0a3, 0x9b2278, 0x95e21a, 
    0x912616, 0x8cf452, 0x89520c, 0x8643c9, 0x83cd5b, 0x81f1d3, 
    0x80b383, 0x8013f8, 0x8013f8, 0x80b383, 0x81f1d3, 0x83cd5b, 
    0x8643c9, 0x89520c, 0x8cf452, 0x912616, 0x95e21a, 0x9b2278, 
    0xa0e0a3, 0xa71570, 0xadb924, 0xb4c375, 0xbc2b9c, 0xc3e85c, 
    0xcbf00f, 0xd438b0, 0xdcb7eb, 0xe56327, 0xee2f94, 0xf71239 };

static void play_audio()
{
    uint64_t timeout = neorv32_clint_time_get() + (uint64_t)(5 * NEORV32_SYSINFO->CLK);
    while (neorv32_clint_time_get() < timeout)
    {
        for (size_t i = 0; i < sin_buf_len; i++)
        {
            // Wait till there's space in the FIFO
            while (I2S_REG_STAT0 & STAT0_FIFO_FULL) {}

            I2S_REG_AUDIOL = sin_buf[i];
            I2S_REG_AUDIOR = sin_buf[i] | AUDIO_COMMIT;
        }
    }
}

static void test_dac()
{
    neorv32_uart0_printf("* DAC Test:\n  => Playing five seconds of audio \n");

    I2S_REG_CTRL0 = CTRL0_RST | CTRL0_DAC_BUILTIN | CTRL0_DAC_EN;
    I2S_REG_CTRL0 &= ~(CTRL0_RST | CTRL0_I2S_EN);
    play_audio();
    I2S_REG_CTRL0 = 0;
    
    neorv32_uart0_printf("  => [DONE]\n");
}

static void test_i2s()
{
    neorv32_uart0_printf("* I2S Test:\n  => Playing five seconds of audio \n");

    I2S_REG_CTRL0 = CTRL0_RST | CTRL0_I2S_EN;
    I2S_REG_CTRL0 &= ~(CTRL0_RST | CTRL0_DAC_BUILTIN | CTRL0_DAC_EN);
    play_audio();
    I2S_REG_CTRL0 = 0;

    neorv32_uart0_printf("  => [DONE]\n");
}

static void test_jtag()
{
    neorv32_uart0_printf("* JTAG Test:\n  => Can't test here, test using OpenOCD instead\n  => [SKIPPED]\n");
}

static void test_oscillator()
{
    neorv32_uart0_printf("* External Oscillator Test:\n  => SoC is running, assuming fine\n  => [OK]\n");
}

static void test_xip_flash()
{
    void *pc;
    __asm__ volatile (
        "auipc %0, 0"
        : "=r"(pc)
    );

    if (((uint32_t)pc & 0xF0000000) == 0xE0000000)
        neorv32_uart0_printf("* XIP Flash Test:\n  => Code is running from XIP (pc = %p)\n  => [OK]\n", pc);
    else
        neorv32_uart0_printf("* XIP Flash Test:\n  => Code is not running from XIP (pc = %p)\n  => [FAIL]\n", pc);
}

int main()
{
    psoc_board_setup(true);
    neorv32_rte_setup();

    neorv32_uart0_printf("\n\n#=========================================================#\n");
    neorv32_uart0_printf("#                    PSoC Board Test                      #\n");
    neorv32_uart0_printf("#=========================================================#\n\n\n");

    test_oscillator();
    test_xip_flash();
    test_jtag();
    test_i2c_oled();
    test_sd_card();
    test_led();
    test_i2s();
    test_dac();
    test_uart();
    test_buttons();
    test_rst_button();

    while (true) {}
    return 0;
}