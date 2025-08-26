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

    enum class PlayerOutput
    {
        I2S,
        DAC
    };

    class Display {
    private:
        Adafruit_SSD1306 _display;
        bool _needUpdate = true;

        string<64> _text;
        DisplayMode _mode;
        bool _playing;
        bool _loopFile;
        size_t _samplesTotal, _samples;
        PlayerOutput _output;

        void drawNumber(size_t val);

    public:
        bool init();
        void update();
        void showText(istring& text)
        {
            _mode = DisplayMode::text;
            _text = text;
            _needUpdate = true;
        }

        void showPlayer()
        {
            _mode = DisplayMode::player;
            _needUpdate = true;
        }

        void setPlayerTitle(istring& text)
        {
            _text = text;
            _needUpdate = true;
        }

        void setPlayerOutput(PlayerOutput output)
        {
            _output = output;
            _needUpdate = true;
        }

        void setPlayerPlaying(bool playing)
        {
            _playing = playing;
            _needUpdate = true;
        }

        void setPlayerTotalSamples(size_t samples)
        {
            _samplesTotal = samples;
            _needUpdate = true;
        }

        void setPlayerSamples(size_t samples)
        {
            _samples = samples;
            _needUpdate = true;
        }

        void setPlayerLoop(bool loopFile)
        {
            _loopFile = loopFile;
            _needUpdate = true;
        }
    };
};