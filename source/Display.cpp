#include "psoc/Display.h"
#include "psoc/splash.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 32
#define DISPLAY_ADDRESS 0x3C
#define FONT_WIDTH 5
#define FONT_HEIGHT 7

namespace psoc {

    bool Display::init()
    {
        _display = Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);
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

                _display.setCursor(0, DISPLAY_HEIGHT/2);

                for (size_t i = 0; i < _text.length(); i++)
                    _display.write(_text[i]);

                break;
            }
            case DisplayMode::player:
            {
                break;
            }
        }

        _display.display();
        _needUpdate = false;
    }

    void Display::showText(istring& text)
    {
        _mode = DisplayMode::text;
        _text = text;
        _needUpdate = true;
    }
}