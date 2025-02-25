#ifndef EDITOR_H
#define EDITOR_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include "viewinitialization.h"

class TileAtlas;
class TileMap;
class UI;

class Editor {
private:
    // main window to render and draw to
    sf::RenderWindow window;
    sf::ContextSettings settings;

    // separate views to split up interaction and rendering
    sf::View uiView;
    sf::View layerView;
    sf::View atlasView;
    sf::View separatorView;

    // store original view size so it isn't overwritten and replaced when zooming
    sf::Vector2f atlasOriginalViewSize;
    sf::Vector2f layerOriginalViewSize;

    // rectangle objects to split up viewports visually
    sf::RectangleShape verticalSeparator, horizontalSeparator;

    // variable to prevent too many inputs registering each frame
    float inputDelay = 0.05f;

    // use pointer to these classes to avoid circular dependencies
    std::shared_ptr<UI> ui;
    std::shared_ptr<TileMap> tileMap;
    std::shared_ptr<TileAtlas> tileAtlas;

public:
    // variables to track zooming
    const std::vector<int> zoomLevels = { 1, 4, 8 };    // zoom multiples
    int currentZoomIndex = 0;  // start at the default zoom level
    float atlasScaleFactor = 1.0f;
    float layerScaleFactor = 1.0f;
    const float baseTileSize = 16.0f; // const tile size for zooming

    // variables to track panning offsets
    sf::Vector2f atlasViewOffset = { 0.f, 0.f };
    sf::Vector2f layerViewOffset = { 0.f, 0.f };

    // main editor functions
    Editor();
    void InitializeClass();
    void Run();
    void Render(sf::RenderWindow& window);

    // main event handling and input processing
    void HandleResize(const sf::Event& event);
    void HandleEvents(float deltaTime);
    void ProcessKeyboardInputs();

    // view-specific event handling
    void HandleAtlasEvents(const sf::Event& event, const sf::Vector2f& atlasMousePos,
        float deltaTime, bool isRightDragging, bool isMiddleDragging);
    void HandleLayerEvents(const sf::Event& event, const sf::Vector2f& layerMousePos,
        float deltaTime, bool isLeftDragging, bool isRightDragging, bool isMiddleDragging);
    void HandleUIEvents(const sf::Event& event, const sf::Vector2f& uiMousePos,
        float deltaTime);
    // zoom event handling
    void HandleAtlasZoom(sf::View& view, float delta,
        const sf::Vector2f& originalSize);
    void HandleLayerZoom(sf::View& view, float delta,
        const sf::Vector2f& originalSize);

    // bounds getter function for views
    sf::FloatRect GetViewportBounds(const sf::View& view,
        const sf::RenderWindow& window);

    // getter functions
    sf::View GetUIView() const { return uiView; }
    sf::View GetAtlasView() const { return atlasView; }
    sf::View GetLayerView() const { return layerView; }
    const sf::RenderWindow& GetWindow() { return window; }
    std::shared_ptr<TileMap> GetTileMap() const { return tileMap; }
};
#endif // !EDITOR_H