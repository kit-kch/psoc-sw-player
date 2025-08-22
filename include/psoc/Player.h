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
        PlayerState _state = PlayerState::splash;
        sd_card_t _cardInfo;
        FATFS _fs;
        FIL _file;

        bool initSDCard();
        void initI2S();

    public:
        void init();
        void run();
    };
};