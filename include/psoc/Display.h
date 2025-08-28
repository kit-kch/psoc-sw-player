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

    enum class DisplayUpdate {
        none,
        all,
        playTime
    };

    class Display {
    private:
        Adafruit_SSD1306 _display;
        uint8_t _displayBuffer[512];
        DisplayUpdate _needUpdate;

        string<64> _text;
        DisplayMode _mode;
        bool _playing;
        uint8_t _volume;
        bool _loopFile;
        size_t _samplesTotal, _samples;
        PlayerOutput _output;

        void drawNumber(size_t val);
        void renderPlayTime();
        void renderPlayer();

    public:
        bool init();
        void update();
        void showText(istring& text)
        {
            _mode = DisplayMode::text;
            _text = text;
            _needUpdate = DisplayUpdate::all;
        }

        void showPlayer()
        {
            _mode = DisplayMode::player;
            _needUpdate = DisplayUpdate::all;
        }

        void setPlayerTitle(istring& text)
        {
            _text = text;
            _needUpdate = DisplayUpdate::all;
        }

        void setPlayerOutput(PlayerOutput output)
        {
            _output = output;
            _needUpdate = DisplayUpdate::all;
        }

        void setPlayerPlaying(bool playing)
        {
            _playing = playing;
            _needUpdate = DisplayUpdate::all;
        }

        void setPlayerTotalSamples(size_t samples)
        {
            // Scale the samples here, so we only have to do it once
            _samplesTotal = samples / 126;
            _needUpdate = DisplayUpdate::playTime;
        }

        void setPlayerSamples(size_t samples)
        {
            _samples = samples;
            _needUpdate = DisplayUpdate::playTime;
        }

        void setPlayerLoop(bool loopFile)
        {
            _loopFile = loopFile;
            _needUpdate = DisplayUpdate::all;
        }

        void setPlayerVolume(uint8_t volume)
        {
            _volume = volume;
            _needUpdate = DisplayUpdate::all;
        }
    };
};