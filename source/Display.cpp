#include "psoc/Display.h"
#include "psoc/splash.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32
#define DISPLAY_ADDRESS 0x3C
// Actually x advance / y advance
#define FONT_WIDTH 6
#define FONT_HEIGHT 8

namespace psoc {

    static void samplesToMS(size_t samples, size_t& mins, size_t& secs)
    {
        secs = samples / 48000;
        mins = secs / 60;
        secs = secs % 60;
    }

    void Display::drawNumber(size_t val)
    {
        size_t d1 = (val / 10) % 10;
        size_t d0 = val % 10;

        _display.write(d1 + '0');
        _display.write(d0 + '0');
    }

    bool Display::init()
    {
        _display = Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);
        _output = PlayerOutput::I2S;
        _samples = 0;
        _samplesTotal = 1;
        _playing = false;
        _loopFile = false;
        _text = "";
        _mode = DisplayMode::splash;
        return _display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS);
    }

    void Display::update()
    {
        if (!_needUpdate)
            return;

        switch (_mode)
        {
            case DisplayMode::splash:
            {
                _display.clearDisplay();
                _display.drawBitmap(0, 0, psoc_splash_data, psoc_splash_width, psoc_splash_height, 1);
                break;
            }
            case DisplayMode::text:
            {
                _display.clearDisplay();
                _display.setTextSize(1);
                _display.setTextColor(SSD1306_WHITE);

                // Center the text
                size_t offset = ((128/FONT_WIDTH) - _text.length()) * FONT_WIDTH/2;
                if (offset > 128)
                    offset = 0;
                _display.setCursor(offset, DISPLAY_HEIGHT/2);

                for (size_t i = 0; i < _text.length(); i++)
                    _display.write(_text[i]);

                break;
            }
            case DisplayMode::player:
            {
                _display.clearDisplay();

                // PSOC Logo
                _display.setTextSize(1);
                _display.setTextColor(SSD1306_WHITE);
                _display.setCursor(0, 0);
                _display.write('P');
                _display.write('S');
                _display.write('o');
                _display.write('C');

                // Replay indicator
                size_t loopLeft = 127 - 4*FONT_WIDTH-2-11;
                if (_loopFile)
                    _display.drawRect(loopLeft, 1, 8, 5, SSD1306_WHITE);

                // Play / Pause Indicator
                size_t playStateLeft = 127 - 4*FONT_WIDTH-2;
                if (_playing)
                {
                    _display.fillTriangle(playStateLeft, 0, playStateLeft, 6, playStateLeft + 4, 3, SSD1306_WHITE);
                }
                else
                {
                    _display.fillRect(playStateLeft, 0, 2, 7, SSD1306_WHITE);
                    _display.fillRect(playStateLeft+3, 0, 2, 7, SSD1306_WHITE);
                }

                // I2S/DAC Mode
                _display.setCursor(127 - 3*FONT_WIDTH, 0);
                if (_output == PlayerOutput::I2S)
                {
                    _display.write('I');
                    _display.write('2');
                    _display.write('S');
                }
                else
                {
                    _display.write('D');
                    _display.write('A');
                    _display.write('C');
                }

                // File name
                _display.setCursor(0, 15);
                size_t titleMax = _text.length() < 12 ? _text.length() : 12;
                for (size_t i = 0; i < titleMax; i++)
                    _display.write(_text[i]);

                // Playing time
                size_t playTimeLeft = 128 - 5*FONT_WIDTH;
                _display.setCursor(playTimeLeft, 15);
                size_t mins, secs;
                samplesToMS(_samples, mins, secs);
                drawNumber(mins);
                _display.write(':');
                drawNumber(secs);

                // Time progress bar
                _display.drawRect(0, 28, 127, 4, SSD1306_WHITE);
                size_t progress = (126 * _samples) / _samplesTotal;
                _display.fillRect(1, 29, progress, 2, SSD1306_WHITE);

                break;
            }
        }

        _display.display();
        _needUpdate = false;
    }
}