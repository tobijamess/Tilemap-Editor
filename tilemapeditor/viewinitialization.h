#ifndef VIEW_INITIALIZATION_H
#define VIEW_INITIALIZATION_H

#include <SFML/Graphics.hpp>

const void InitializeUIView(sf::View& uiView, const sf::RenderWindow& window);
const void InitializeAtlasView(sf::View& atlasView, sf::Vector2f& atlasOriginalViewSize,
    const sf::RenderWindow& window);
const void InitializeLayerView(sf::View& layerView, sf::Vector2f& layerOriginalViewSize,
    const sf::RenderWindow& window);
void InitializeSeparatorView(sf::View& separatorView,
    sf::RectangleShape& verticalSeparator,
    sf::RectangleShape& horizontalSeparator,
    const sf::RenderWindow& window,
    const sf::View& atlasView,
    const sf::View& layerView);

#endif // VIEW_INITIALIZATION_H
