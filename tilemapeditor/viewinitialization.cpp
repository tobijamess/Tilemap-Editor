#include "viewinitialization.h"

const void InitializeUIView(sf::View& uiView, const sf::RenderWindow& window) {
    const float uiViewportWidth = 0.75f;  // 75% of window width
    const float uiViewportHeight = 0.25f; // 25% of window height
    uiView.setViewport(sf::FloatRect(0.25f, 0.75f, uiViewportWidth, uiViewportHeight));

    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    const float uiAspectRatio = (windowWidth * uiViewportWidth) / (windowHeight * uiViewportHeight);

    uiView.setSize(windowWidth * uiViewportWidth, (windowWidth * uiViewportWidth) / uiAspectRatio);
    uiView.setCenter(uiView.getSize() / 2.f);
}

const void InitializeAtlasView(sf::View& atlasView, sf::Vector2f& atlasOriginalViewSize, const sf::RenderWindow& window) {
    const float atlasViewportWidth = 0.25f; // 25% of window width
    const float atlasViewportHeight = 1.0f; // 100% of window height
    atlasView.setViewport(sf::FloatRect(0.f, 0.f, atlasViewportWidth, atlasViewportHeight));

    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    const float atlasAspectRatio = (windowWidth * atlasViewportWidth) / (windowHeight * atlasViewportHeight);

    atlasView.setSize(windowWidth * atlasViewportWidth, (windowWidth * atlasViewportWidth) / atlasAspectRatio);
    atlasView.setCenter(atlasView.getSize() / 2.f);
    atlasOriginalViewSize = atlasView.getSize(); // store original size for zoom functions
}

const void InitializeLayerView(sf::View& layerView, sf::Vector2f& layerOriginalViewSize, const sf::RenderWindow& window) {
    const float layerViewportWidth = 0.75f; // 75% of window width
    const float layerViewportHeight = 0.75f; // 75% of window height
    layerView.setViewport(sf::FloatRect(0.25f, 0.f, layerViewportWidth, layerViewportHeight));

    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    const float layerAspectRatio = (windowWidth * layerViewportWidth) / (windowHeight * layerViewportHeight);

    layerView.setSize(windowWidth * layerViewportWidth, windowWidth * layerViewportWidth / layerAspectRatio);
    layerView.setCenter(layerView.getSize() / 2.f);
    layerOriginalViewSize = layerView.getSize(); // store original size for zoom functions
}

void InitializeSeparatorView(sf::View& separatorView,
    sf::RectangleShape& verticalSeparator,
    sf::RectangleShape& horizontalSeparator,
    const sf::RenderWindow& window,
    const sf::View& atlasView,
    const sf::View& layerView)
{
    // initialize the separator view to cover the entire window
    separatorView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    separatorView.setSize(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y));
    separatorView.setCenter(separatorView.getSize().x / 2.f, separatorView.getSize().y / 2.f);

    // vertical separator:
    // 2px wide and spans the full height of the window
    verticalSeparator.setSize(sf::Vector2f(2.f, static_cast<float>(window.getSize().y)));
    verticalSeparator.setFillColor(sf::Color::White);
    // place it at the right edge of the atlas view's viewport
    verticalSeparator.setPosition(atlasView.getViewport().width * window.getSize().x, 0.f);

    // horizontal separator:
    // 2px tall and spans the full width of the window
    horizontalSeparator.setSize(sf::Vector2f(static_cast<float>(window.getSize().x), 2.f));
    horizontalSeparator.setFillColor(sf::Color::White);
    // place it at the bottom edge of the layer view's viewport
    horizontalSeparator.setPosition(atlasView.getViewport().width * window.getSize().x,
        layerView.getViewport().height * window.getSize().y);
}
