#include "DemoText.h"

#include <fstream>

bool loadDemoFont(sf::Font &font)
{
    const char *paths[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
    };
    for (const char *path : paths)
    {
        if (std::ifstream(path).good() && font.openFromFile(path))
        {
            return true;
        }
    }
    return false;
}

void drawDemoText(sf::RenderWindow &window, const sf::Font &font,
                  const std::string &text, float x, float y, unsigned size,
                  sf::Color color)
{
    sf::Text label(font, text, size);
    label.setFillColor(color);
    label.setPosition({x, y});
    window.draw(label);
}
