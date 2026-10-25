#ifndef DEMO_TEXT_H
#define DEMO_TEXT_H

#include <SFML/Graphics.hpp>

#include <string>

/**
 * @brief Loads a font that is already on the computer (Windows, Linux or Mac).
 * @param font The font to load into.
 * @return true if a font was found.
 */
bool loadDemoFont(sf::Font &font);

/**
 * @brief Writes one line of text with its top-left corner at (x, y).
 */
void drawDemoText(sf::RenderWindow &window, const sf::Font &font,
                  const std::string &text, float x, float y, unsigned size,
                  sf::Color color);

#endif
