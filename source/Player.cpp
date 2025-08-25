#include "psoc/Player.h"

#include <neorv32.h>
#include <psoc_i2s.h>
#include <neosd.h>
#include <ff.h>

static uint64_t neorv32_clint_time_get_ms()
{
    return (neorv32_clint_time_get() / ((uint64_t)neorv32_sysinfo_get_clk()))*1000;
}

namespace psoc {

    bool Player::initSDCard()
    {
        neosd_setup(CLK_PRSC_1024, 0, 0);
        neorv32_uart0_printf("NEOSD: Controller initialized\n");

        switch (neosd_app_card_init(&_cardInfo))
        {
            case NEOSD_OK:
                neorv32_uart0_printf("SD Card initialized: CCS: %d, UHS2: %d, S18A: %d, OCR: %x\n",
                    _cardInfo.ccs, _cardInfo.uhs2, _cardInfo.s18a, _cardInfo.ocr);
                neorv32_uart0_printf("    Manufacturer: %x, OID: %c%c, Product: %c%c%c%c%c\n",
                    _cardInfo.cid.mid, _cardInfo.cid.oid[1], _cardInfo.cid.oid[0], _cardInfo.cid.pnm[4],
                    _cardInfo.cid.pnm[3], _cardInfo.cid.pnm[2], _cardInfo.cid.pnm[1], _cardInfo.cid.pnm[0]);
                neorv32_uart0_printf("    Revision: %d.%d, Serial: %u, Date: %d/%d\n",
                    _cardInfo.cid.prv >> 4, _cardInfo.cid.prv & 0xF, _cardInfo.cid.psn, _cardInfo.cid.mdt & 0xF,
                    (_cardInfo.cid.mdt >> 4) + 2000);

                if (!neosd_app_configure_datamode(false, _cardInfo.rca))
                {
                    neorv32_uart0_printf("Error: Couldn't configure 1-bit data mode for SD Card!\n");
                    return false;
                }

                // Fast clock for data transfer
                neosd_set_clock_div(CLK_PRSC_8, 0);

                return true;
            case NEOSD_NO_CARD:
                neorv32_uart0_printf("No SD Card found\n");
                return false;
            case NEOSD_INCOMPAT_CARD:
                neorv32_uart0_printf("Inserted SD Card is not compatible\n");
                return false;
            case NEOSD_CRC_ERR:
                neorv32_uart0_printf("CRC error during communication\n");
                return false;
            case NEOSD_TIMEOUT:
                neorv32_uart0_printf("Timeout during communication\n");
                return false;
            default:
                return false;
        }
    }

    void Player::initI2S()
    {
        I2S_REG_CTRL0 |= CTRL0_RST | CTRL0_I2S_EN;
        I2S_REG_CTRL0 &= ~(CTRL0_RST | CTRL0_DAC_BUILTIN | CTRL0_DAC_EN);
    }

    void Player::init()
    {
        _display.init();
        initI2S();
    }

    void Player::run()
    {
        auto startTime = neorv32_clint_time_get_ms();
        string<64> msg = "Initializing SD Card";

        neorv32_uart0_printf("Showing splash screen for 5s...\n");
        while (true)
        {
            switch (_state)
            {
                case PlayerState::splash:
                {
                    if (neorv32_clint_time_get_ms() > startTime + 5000)
                    {
                        _state = PlayerState::initSD;
                        _display.showText(msg);
                    }
                    break;
                }
                case PlayerState::initSD:
                {
                    if (!initSDCard())
                    {
                        msg = "SD Card Fail";
                        _display.showText(msg);
                        _state = PlayerState::errorSD;
                        break;
                    }

                    neorv32_uart0_printf("Mounting SD Card\n");
                    if (f_mount(&_fs, "", 1) != FR_OK)
                    {
                        msg = "Mounting FS failed";
                        _display.showText(msg);
                        _state = PlayerState::errorSD;
                        neorv32_uart0_printf("Mounting SD Card failed\n");
                        break;
                    }
                    neorv32_uart0_printf("Mounting SD Card ok\n");

                    // Open file
                    neorv32_uart0_printf("Opening fest2019.s16\n");
                    if (f_open(&_file, "fest2019.s16", FA_OPEN_EXISTING | FA_READ) != FR_OK)
                    {
                        neorv32_uart0_printf("Opening fest2019.s16 failed\n");
                        msg = "File not found";
                        _display.showText(msg);
                        _state = PlayerState::errorSD;
                        break;
                    }
                    neorv32_uart0_printf("Opening fest2019.s16 ok\n");
                    
                    auto size = f_size(&_file);
                    neorv32_uart0_printf("File size is: %d\n", size);

                    msg = "Playing";
                    _display.showText(msg);
                    _state = PlayerState::play;

                    /*
                    if (f_close(&file) != FR_OK)
                    {
                        neorv32_uart0_printf("Closing fest2019.s16 failed\n");
                        msg = "File close failed";
                        _display.showText(msg);
                        _state = PlayerState::errorSD;
                        break;
                    }
                    */
                    break;
                }
                case PlayerState::errorSD:
                {
                    break;
                }
                case PlayerState::play:
                {
                    UINT numRead;
                    int16_t audioBuf[64];

                    while (true)
                    {
                        if (f_read(&_file, &audioBuf[0], 128, &numRead) != FR_OK)
                        {
                            msg = "Error reading file";
                            _display.showText(msg);
                            _state = PlayerState::errorSD;
                            neorv32_uart0_printf("Reading fest2019.s16 failed\n");
                            break;
                        }

                        for (size_t i = 0; i < 64 / 2; i++)
                        {
                            while (I2S_REG_STAT0 & STAT0_FIFO_FULL) {}

                            // Sign extend to 32 bit
                            int32_t left = ((int32_t)audioBuf[i*2]) << 16;
                            left >>= 10;
                            int32_t right = ((int32_t)audioBuf[i*2 + 1]) << 16;
                            right >>= 10;
                            // Audio data is actually 24 bit. Lower 24 bits are in correct format.
                            // Top bits are ignored anyway, except for commit, which must be cleared.
                            I2S_REG_AUDIOL = left & ~AUDIO_COMMIT;
                            I2S_REG_AUDIOR = right | AUDIO_COMMIT;
                        }
                    }

                    break;
                }
            }
            _display.update();
        }
    }
}