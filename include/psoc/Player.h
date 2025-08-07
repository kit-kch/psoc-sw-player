#pragma once

#include <etl/string.h>
#include <psoc/Display.h>

using etl::string, etl::istring;

namespace psoc {
    enum class PlayerState
    {
        splash,
        initSD
    };

    class Player {
    private:
        Display _display;
        PlayerState _state = PlayerState::splash;

    public:
        void init();
        void run();
    };
};