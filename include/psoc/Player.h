#pragma once

#include <etl/string.h>
#include <psoc/Display.h>

#include <neosd_app.h>
#include <ff.h>

using etl::string, etl::istring;

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
        DIR _dir;
        uint32_t _buttonLast;
        size_t _audioIndex;
        uint32_t _audioBuf[32];
        string<64> msg;

        bool _playing;
        PlayerOutput _output;
        size_t _samplesPlayed;
        bool _fileFinished;

        bool initSDCard();
        uint32_t buttonRising();
        bool handleInputs();
        bool openFile(const char* name);
        bool selectNextFile();
        void playAudio();

    public:
        void init();
        void run();
    };
};