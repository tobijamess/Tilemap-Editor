#include "viewinitialization.h"

void InitializeUIView(sf::View& uiView, const sf::RenderWindow& window) {
    float uiViewportWidth = 0.75f;  // 75% of window width
    float uiViewportHeight = 0.25f; // 25% of window height
    uiView.setViewport(sf::FloatRect(0.25f, 0.75f, uiViewportWidth, uiViewportHeight));

    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    float uiAspectRatio = (windowWidth * uiViewportWidth) / (windowHeight * uiViewportHeight);

    uiView.setSize(windowWidth * uiViewportWidth, (windowWidth * uiViewportWidth) / uiAspectRatio);
    uiView.setCenter(uiView.getSize() / 2.f);
}

void InitializeAtlasView(sf::View& atlasView, sf::Vector2f& atlasOriginalViewSize, const sf::RenderWindow& window) {
    float atlasViewportWidth = 0.25f; // 25% of window width
    float atlasViewportHeight = 1.0f; // 100% of window height
    atlasView.setViewport(sf::FloatRect(0.f, 0.f, atlasViewportWidth, atlasViewportHeight));

    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    float atlasAspectRatio = (windowWidth * atlasViewportWidth) / (windowHeight * atlasViewportHeight);

    atlasView.setSize(windowWidth * atlasViewportWidth, (windowWidth * atlasViewportWidth) / atlasAspectRatio);
    atlasView.setCenter(atlasView.getSize() / 2.f);
    atlasOriginalViewSize = atlasView.getSize(); // store original size for zoom functions
}

void InitializeLayerView(sf::View& layerView, sf::Vector2f& layerOriginalViewSize, const sf::RenderWindow& window) {
    float layerViewportWidth = 0.75f; // 75% of window width
    float layerViewportHeight = 0.75f; // 75% of window height
    layerView.setViewport(sf::FloatRect(0.25f, 0.f, layerViewportWidth, layerViewportHeight));

    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    float layerAspectRatio = (windowWidth * layerViewportWidth) / (windowHeight * layerViewportHeight);

    layerView.setSize(windowWidth * layerViewportWidth, windowWidth * layerViewportWidth / layerAspectRatio);
    layerView.setCenter(layerView.getSize() / 2.f);
    layerOriginalViewSize = layerView.getSize(); // store original size for zoom functions
}

void InitializeViewSeparators(sf::RectangleShape& verticalSeparator, sf::RectangleShape& horizontalSeparator,
    const sf::View& atlasView, const sf::View& layerView, const sf::RenderWindow& window) {
    // vertical separator
    verticalSeparator.setSize(sf::Vector2f(2.0f, static_cast<float>(window.getSize().y))); // 2px wide line
    verticalSeparator.setFillColor(sf::Color::White);
    verticalSeparator.setPosition(atlasView.getViewport().width * window.getSize().x, 0.0f);

    // horizontal separator
    horizontalSeparator.setSize(sf::Vector2f(static_cast<float>(window.getSize().x), 2.0f)); // 2px tall line
    horizontalSeparator.setFillColor(sf::Color::White);
    horizontalSeparator.setPosition(atlasView.getViewport().width * window.getSize().x,
        layerView.getViewport().height * window.getSize().y);
}