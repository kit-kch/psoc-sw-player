#include "psoc/Player.h"

#include <neorv32.h>
#include <psoc_i2s.h>
#include <psoc_board.h>
#include <neosd.h>
#include <ff.h>

namespace psoc {

    Player* gPlayer;
    // Only using this for the audio fifo low pin, so no need to check source
    void gpio_interrupt_handler()
    {
        neorv32_gpio_irq_clr(neorv32_gpio_irq_get());
        gPlayer->playAudio();
    }

    static uint64_t neorv32_clint_time_get_ms()
    {
        return (neorv32_clint_time_get() / ((uint64_t)neorv32_sysinfo_get_clk()))*1000;
    }

    static void queueAudioS16(int16_t left, int16_t right, uint8_t volumeShift)
    {
        // Sign extend to 32 bit
        int32_t left32 = ((int32_t)left) << 16;
        left32 >>= volumeShift;
        int32_t right32 = ((int32_t)right) << 16;
        right32 >>= volumeShift;
        // Audio data is actually 24 bit. Lower 24 bits are in correct format.
        // Top bits are ignored anyway, except for commit, which must be cleared.
        I2S_REG_AUDIOL = left32 & ~AUDIO_COMMIT;
        I2S_REG_AUDIOR = right32 | AUDIO_COMMIT;
    }

    bool Player::initSDCard()
    {
        neorv32_uart0_printf("Initializing NEOSD controller\n");
        neosd_setup(CLK_PRSC_1024, 0, 0);
        neorv32_uart0_printf("  => ok\n\n");

        neorv32_uart0_printf("Initializing SD card\n");
        switch (neosd_app_card_init(&_cardInfo))
        {
            case NEOSD_OK:
                neorv32_uart0_printf("  => ok\n");
                neorv32_uart0_printf("     CCS: %d, UHS2: %d, S18A: %d, OCR: %x\n",
                    _cardInfo.ccs, _cardInfo.uhs2, _cardInfo.s18a, _cardInfo.ocr);
                neorv32_uart0_printf("     Manufacturer: %x, OID: %c%c, Product: %c%c%c%c%c\n",
                    _cardInfo.cid.mid, _cardInfo.cid.oid[1], _cardInfo.cid.oid[0], _cardInfo.cid.pnm[4],
                    _cardInfo.cid.pnm[3], _cardInfo.cid.pnm[2], _cardInfo.cid.pnm[1], _cardInfo.cid.pnm[0]);
                neorv32_uart0_printf("     Revision: %d.%d, Serial: %u, Date: %d/%d\n\n",
                    _cardInfo.cid.prv >> 4, _cardInfo.cid.prv & 0xF, _cardInfo.cid.psn, _cardInfo.cid.mdt & 0xF,
                    (_cardInfo.cid.mdt >> 4) + 2000);

                neorv32_uart0_printf("Setting 1-bit mode\n");
                if (!neosd_app_configure_datamode(false, _cardInfo.rca))
                {
                    neorv32_uart0_printf("  => Error: Couldn't configure 1-bit data mode for SD Card!\n");
                    return false;
                }
                neorv32_uart0_printf("  => ok\n\n");

                // Fast clock for data transfer
                neorv32_uart0_printf("Increasing SD clock\n");
                neosd_set_clock_div(CLK_PRSC_2, 0);
                neorv32_uart0_printf("  => ok\n\n");

                return true;
            case NEOSD_NO_CARD:
                neorv32_uart0_printf("  => No SD Card found\n");
                return false;
            case NEOSD_INCOMPAT_CARD:
                neorv32_uart0_printf("  => Inserted SD Card is not compatible\n");
                return false;
            case NEOSD_CRC_ERR:
                neorv32_uart0_printf("  => CRC error during communication\n");
                return false;
            case NEOSD_TIMEOUT:
                neorv32_uart0_printf("  => Timeout during communication\n");
                return false;
            default:
                return false;
        }
    }

    static void enableAudioInterrupt()
    {
        //neorv32_cpu_csr_set(CSR_MIE, 1 << GPIO_FIRQ_ENABLE);
        neorv32_gpio_irq_enable(1 << GPIO_I2S_IRQ);
    }

    static void disableAudioInterrupt()
    {
        //neorv32_cpu_csr_clr(CSR_MIE, 1 << GPIO_FIRQ_ENABLE);
        neorv32_gpio_irq_disable(1 << GPIO_I2S_IRQ);
    }

    static void initAudio()
    {
        neorv32_rte_handler_install(GPIO_RTE_ID, gpio_interrupt_handler);
        neorv32_cpu_csr_set(CSR_MIE, 1 << GPIO_FIRQ_ENABLE);
        neorv32_cpu_csr_set(CSR_MSTATUS, 1 << CSR_MSTATUS_MIE);
        neorv32_gpio_irq_setup(GPIO_I2S_IRQ, GPIO_TRIG_LEVEL_HIGH);
        neorv32_gpio_irq_enable(1 << GPIO_I2S_IRQ);

        I2S_REG_LOWT = 96;
        I2S_REG_CTRL0 = CTRL0_RST | CTRL0_I2S_EN;
        I2S_REG_CTRL0 = CTRL0_I2S_EN;
    }

    static void setAudioOutput(PlayerOutput output)
    {
        if (output == PlayerOutput::I2S)
            I2S_REG_CTRL0 = CTRL0_I2S_EN;
        else
            I2S_REG_CTRL0 = CTRL0_DAC_BUILTIN | CTRL0_DAC_EN;
    }

    uint32_t Player::buttonRising()
    {
        uint32_t buttons = neorv32_gpio_port_get();
        // Last = high, new = low. This is falling, but the buttons are low-active
        uint32_t rising = _buttonLast & ~buttons;
        _buttonLast = buttons;
        return rising;
    }

    bool Player::handleInputs()
    {
        auto buttons = buttonRising();

        if (buttons & (1 << PSOC_BTN_SW1))
        {
            if (_output == PlayerOutput::I2S)
                _output = PlayerOutput::DAC;
            else
                _output = PlayerOutput::I2S;
            
            _display.setPlayerOutput(_output);
            setAudioOutput(_output);
        }
        if (buttons & (1 << PSOC_BTN_L))
        {
            _loopFile = !_loopFile;
            _display.setPlayerLoop(_loopFile);
        }
        if (buttons & (1 << PSOC_BTN_U))
        {
            if (_volume < 16)
            {
                _volume++;
                _display.setPlayerVolume(_volume);
                _volumeShift = 15 - _volume;
            }
        }
        if (buttons & (1 << PSOC_BTN_D))
        {
            if (_volume > 0)
            {
                _volume--;
                _display.setPlayerVolume(_volume);
                _volumeShift = 15 - _volume;
            }
        }
        if (buttons & (1 << PSOC_BTN_R))
        {
            if (!selectNextFile())
                return false;
            if (!openFile(msg.c_str()))
                return false;
        }
        if (buttons & (1 << PSOC_BTN_C))
        {
            _playing = !_playing;
            _display.setPlayerPlaying(_playing);
        }
        return true;
    }

    // Note: this is called from gpio_interrupt_handler IRQ
    // So we have to be a bit careful to mark some things volatile here..
    // Note that reading the SD card in an interrupt is probably a really bad idea....
    // But we don't use interrupts anywhere else and don't care if we stall the main programm for some time
    // But audio really needs to be gapless
    void Player::playAudio()
    {
        if (!_playing)
        {
            while (!(I2S_REG_STAT0 & STAT0_FIFO_FULL))
            {
                queueAudioS16(0, 0, _volumeShift);
            }
            return;
        }

        while (!(I2S_REG_STAT0 & STAT0_FIFO_FULL))
        {

            // Refill the buffer
            if (_audioIndex == AUDIO_BUF_SIZE)
            {
                _samplesPlayed += AUDIO_BUF_SIZE;
                UINT numRead;
                if (f_read(&_file, &_audioBuf[0], AUDIO_BUF_SIZE*4, &numRead) != FR_OK)
                {
                    msg = "Error reading file";
                    _display.showText(msg);
                    _state = PlayerState::errorSD;
                    neorv32_uart0_printf("Reading fest2019.s16 failed\n");
                    break;
                }

                // EOF. Don't handle partial buffer, we're done
                if (numRead < AUDIO_BUF_SIZE*4)
                {
                    _playing = false;
                    _fileFinished = true;
                    return;
                }

                _audioIndex = 0;
            }

            queueAudioS16(_audioBuf[_audioIndex] & 0xFFFF, _audioBuf[_audioIndex] >> 16, _volumeShift);
            _audioIndex++;
        }
    }

    bool Player::openFile(const char* name)
    {
        // So that fatfs does not get interrupted
        disableAudioInterrupt();

        // Close old file if open
        if (_fileOpen && f_close(&_file) != FR_OK)
        {
            neorv32_uart0_printf("Closing file failed!\n", name);
            enableAudioInterrupt();
            return false;
        }
        
        // Open file
        neorv32_uart0_printf("Opening %s...\n", name);
        if (f_open(&_file, name, FA_OPEN_EXISTING | FA_READ) != FR_OK)
        {
            neorv32_uart0_printf("  => failed\n");
            enableAudioInterrupt();
            return false;
        }
    
        neorv32_uart0_printf("  => ok\n");
        auto size = f_size(&_file);
        neorv32_uart0_printf("  => size: %d\n\n", size);
    
        // 2 bytes per sample, 2 channels
        _display.setPlayerTotalSamples(size / (2 * 2));
        _samplesPlayed = 0;
        _display.setPlayerSamples(0);

        _playing = true;
        _display.setPlayerPlaying(true);

        msg = name;
        _display.showText(msg);
        _display.showPlayer();
        _state = PlayerState::play;
        _fileOpen = true;
        _fileFinished = false;
        enableAudioInterrupt();
        return true;
    }

    bool Player::resetFile()
    {
        neorv32_uart0_printf("Resetting file...\n");
        // So that fatfs does not get interrupted
        disableAudioInterrupt();
        if (f_rewind(&_file) != FR_OK)
        {
            neorv32_uart0_printf("  => failed\n");
            enableAudioInterrupt();
            return false;
        }
        neorv32_uart0_printf("  => ok\n");

        _playing = true;
        _fileFinished = false;
        _samplesPlayed = 0;
        _display.setPlayerSamples(0);

        enableAudioInterrupt();
        return true;
    }

    bool Player::selectNextFile()
    {
        // So that fatfs does not get interrupted
        disableAudioInterrupt();

        FILINFO finfo;
        bool rewound = false;
        while (true)
        {
            if(f_readdir(&_dir, &finfo) != FR_OK)
            {
                enableAudioInterrupt();
                return false;
            }
    
            // Already last file => start from 0 again
            if (finfo.fname[0] == 0)
            {
                // No files....
                if (rewound)
                {
                    enableAudioInterrupt();
                    return false;
                }
                rewound = true;
                f_rewinddir(&_dir);
                continue;
            }

            if (finfo.fattrib & AM_DIR)
                continue;

            // Only .s16 extension
            size_t nlen = strlen(finfo.fname);
            if (nlen < 4)
                continue;
                
            if (strcmp(finfo.fname + nlen - 4, ".S16") != 0)
                continue;

            msg = finfo.fname;
            enableAudioInterrupt();
            return true;
        }
        // Not reachable
        enableAudioInterrupt();
        return false;
    }

    void Player::init()
    {
        gPlayer = this;
        _buttonLast = 0;
        _audioIndex = 32;
        _playing = false;
        _fileOpen = false;
        _loopFile = false;
        _volume = 7;
        _volumeShift = 8;
        _output = PlayerOutput::I2S;
        _state = PlayerState::splash;
        _display.init();
        initAudio();
    }

    void Player::run()
    {
        auto startTime = neorv32_clint_time_get_ms();
        neorv32_uart0_printf("Showing splash screen for 3s...\n");
        uint64_t displayTime = 0;
        while (true)
        {
            switch (_state)
            {
                case PlayerState::splash:
                {
                    if (neorv32_clint_time_get_ms() > startTime + 3000)
                    {
                        neorv32_uart0_printf("  => ok\n\n");
                        _state = PlayerState::initSD;
                    }
                    break;
                }
                case PlayerState::initSD:
                {
                    if (!initSDCard())
                    {
                        msg = "SD init failed!";
                        _display.showText(msg);
                        _state = PlayerState::errorSD;
                        break;
                    }

                    neorv32_uart0_printf("Mounting SD card\n");
                    if (f_mount(&_fs, "", 1) != FR_OK)
                    {
                        msg = "Mounting FAT failed!";
                        _display.showText(msg);
                        _state = PlayerState::errorSD;
                        neorv32_uart0_printf("  => failed\n");
                        break;
                    }
                    neorv32_uart0_printf("  => ok\n\n");

                    neorv32_uart0_printf("Loading root folder\n");
                    if (f_opendir(&_dir, "") != FR_OK)
                    {
                        neorv32_uart0_printf("  => failed\n");
                        msg = "No folder found!";
                        _display.showText(msg);
                        _state = PlayerState::errorSD;
                        break;
                    }
                    neorv32_uart0_printf("  => ok\n\n");

                    neorv32_uart0_printf("Loading first file\n");
                    if (!selectNextFile())
                    {
                        neorv32_uart0_printf("  => failed. No files?\n");
                        msg = "No file found!";
                        _display.showText(msg);
                        _state = PlayerState::errorSD;
                        break;
                    }

                    if (!openFile(msg.c_str()))
                    {
                        msg = "Opening file failed!";
                        _display.showText(msg);
                        _state = PlayerState::errorSD;
                        break;
                    }
                    break;
                }
                case PlayerState::errorSD:
                {
                    break;
                }
                case PlayerState::play:
                {
                    if (!handleInputs())
                    {
                        msg = "Opening file failed!";
                        _display.showText(msg);
                        _state = PlayerState::errorSD;
                        break;
                    }

                    // In IRQ-less version. Otherwise this is called by IRQ
                    // playAudio();
                    if (_fileFinished)
                    {
                        neorv32_uart0_printf("End of file\n");
                        if (_loopFile)
                        {
                            if (!resetFile())
                            {
                                msg = "File read failed!";
                                _display.showText(msg);
                                _state = PlayerState::errorSD;
                                break;
                            }
                        }
                        else
                        {
                           
                            if (!selectNextFile())
                            {
                                neorv32_uart0_printf("  => failed. No files?\n");
                                msg = "No file found!";
                                _display.showText(msg);
                                _state = PlayerState::errorSD;
                                break;
                            }

                            if (!openFile(msg.c_str()))
                            {
                                msg = "Opening file failed!";
                                _display.showText(msg);
                                _state = PlayerState::errorSD;
                                break;
                            }
                        }
                    }

                    // Ignore botton 16 bits => only update every 2^16 samples, which is slightly less than 1s.
                    auto now = neorv32_clint_time_get_ms();
                    if (now - displayTime >= 1000)
                    {
                        displayTime = now;
                        _display.setPlayerSamples(_samplesPlayed);
                    }
                    break;
                }
                default:
                {

                }
            }
            _display.update();
        }
    }
}