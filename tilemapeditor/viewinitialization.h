#ifndef VIEW_INITIALIZATION_H
#define VIEW_INITIALIZATION_H

#include <SFML/Graphics.hpp>

void InitializeUIView(sf::View& uiView, const sf::RenderWindow& window);
void InitializeAtlasView(sf::View& atlasView, sf::Vector2f& atlasOriginalViewSize,
    const sf::RenderWindow& window);
void InitializeLayerView(sf::View& layerView, sf::Vector2f& layerOriginalViewSize,
    const sf::RenderWindow& window);
void InitializeViewSeparators(sf::RectangleShape& verticalSeparator,
    sf::RectangleShape& horizontalSeparator,
    const sf::View& atlasView, const sf::View& layerView,
    const sf::RenderWindow& window);

#endif // VIEW_INITIALIZATION_H