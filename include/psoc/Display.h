#pragma once

#include <etl/string.h>
#include <Adafruit_SSD1306.h>

using etl::string, etl::istring;

namespace psoc {
    enum class DisplayMode {
        splash,
        text,
        player
    };

    class Display {
    private:
        Adafruit_SSD1306 _display;
        bool _needUpdate = true;

        string<64> _text = "";
        DisplayMode _mode = DisplayMode::splash;

    public:
        bool init();
        void update();
        void showText(istring& text);
    };
};