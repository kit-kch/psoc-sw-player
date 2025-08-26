#pragma once

#include <etl/string.h>
#include <psoc/Display.h>

#include <neosd_app.h>
#include <ff.h>

using etl::string, etl::istring;

#define AUDIO_BUF_SIZE 128

namespace psoc {
    enum class PlayerState
    {
        splash,
        initSD,
        errorSD,
        play
    };

    class Player {
    private:
        Display _display;
        PlayerState _state;
        sd_card_t _cardInfo;
        FATFS _fs;
        FIL _file;
        bool _fileOpen;
        bool _loopFile;
        DIR _dir;
        uint32_t _buttonLast;
        size_t _audioIndex;
        uint32_t _audioBuf[AUDIO_BUF_SIZE];
        uint8_t _volume;
        string<64> msg;

        volatile uint8_t _volumeShift;
        volatile bool _playing;
        PlayerOutput _output;
        volatile size_t _samplesPlayed;
        volatile bool _fileFinished;

        bool initSDCard();
        uint32_t buttonRising();
        bool handleInputs();
        bool openFile(const char* name);
        bool resetFile();
        bool selectNextFile();
        void playAudio();

        friend void gpio_interrupt_handler();

    public:
        bool init();
        void run();
    };
};