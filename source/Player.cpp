#include "psoc/Player.h"

#include <neorv32.h>

static uint64_t neorv32_clint_time_get_ms()
{
    return (neorv32_clint_time_get() / ((uint64_t)neorv32_sysinfo_get_clk()))*1000;
}

namespace psoc {

    void Player::init()
    {
        _display.init();
    }

    void Player::run()
    {
        auto startTime = neorv32_clint_time_get_ms();
        string<64> msg = "Initializing SD Card";

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
                    break;
                }
            }
            _display.update();
        }
    }
}