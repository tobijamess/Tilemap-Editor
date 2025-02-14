#ifndef EDITOR_H
#define EDITOR_H

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include "tileatlas.h"

class UI;
class TileMap;

class Editor {
private:
    sf::RenderWindow window;
    // different views to render the atlas, ui and layers to
    sf::View uiView;
    sf::View layerView;
    sf::View atlasView;
    // original view sizes to base zooming off (otherwise it will set the current view size to the zoomed view size preventing ever zooming out)
    sf::Vector2f atlasOriginalViewSize;
    sf::Vector2f layerOriginalViewSize;
    // rectangle objects to split up viewports visually
    sf::RectangleShape verticalSeparator, horizontalSeparator;
    // variable to prevent too many inputs registering each frame
    float inputDelay = 0.05f;
    // use pointer to these classes to avoid circular dependencies
    UI* ui;
    TileMap* tileMap;
    TileAtlas* tileAtlas;
public:
    // variables to track zooming
    const std::vector<int> zoomLevels = { 1, 4, 8 };  // Adjusted for finer zoom steps
    int currentZoomIndex = 0;  // start at the default zoom level (16)
    float atlasScaleFactor = 1.0f;
    float layerScaleFactor = 1.0f;
    const int baseTileSize = 16; // an unchangable tile size used as a reference for zooming
    // variables to track the panning offset for the atlas and layer
    sf::Vector2f atlasViewOffset = { 0.f, 0.f };
    sf::Vector2f layerViewOffset = { 0.f, 0.f };
    // main editor functions
    Editor();
    void Run();
    void Render(sf::RenderWindow& window);
    void HandleEvents(float deltaTime);
    sf::FloatRect GetViewportBounds(const sf::View& view, const sf::RenderWindow& window);
    void HandleAtlasZoom(sf::View& view, float delta, const sf::Vector2f& originalSize);
    void HandleLayerZoom(sf::View& view, float delta, const sf::Vector2f& originalSize);
    void InitializeClass();
    sf::RenderWindow& GetWindow() { return window; }
    sf::View GetUIView() { return uiView; }
    sf::View GetAtlasView() { return atlasView; }
    sf::View GetLayerView() { return layerView; }
    TileMap* GetTileMap() { return tileMap; }
    float clamp(float value, float min, float max) {
        return std::max(min, std::min(max, value));
    }
};
#endif