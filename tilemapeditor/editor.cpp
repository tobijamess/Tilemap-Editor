#include "editor.h"
#include "ui.h"
#include "layer.h"

// default editor constructor because editor needs to be constructed first, and then i can freely initialize other dependencies e.g. ui
Editor::Editor()
    : window(sf::VideoMode(1920, 1080), "Tile Editor")
{
    
    auto windowWidth = static_cast<float>(window.getSize().x);
    auto windowHeight = static_cast<float>(window.getSize().y);

    // initialize default zoom level to match normal rendering
    currentZoomIndex = 1; // index 1 corresponds to zoom level 16 (normal rendering)
    float defaultZoomFactor = static_cast<float>(zoomLevels[currentZoomIndex]) / zoomLevels[0];

    // ui view initialization
    float uiViewportWidth = 0.75f; // 75% of window width
    float uiViewportHeight = 0.25f; // 25% of window height
    uiView.setViewport(sf::FloatRect(0.25f, 0.75f, 0.75f, 0.25f));
    float uiAspectRatio = (windowWidth * uiViewportWidth) / (windowHeight * uiViewportHeight);
    uiView.setSize(windowWidth * uiViewportWidth, (windowWidth * uiViewportWidth) / uiAspectRatio);
    uiView.setCenter(uiView.getSize() / 2.f); // center the view

    // atlas view initialization
    float atlasViewportWidth = 0.25f; // 25% of window width
    float atlasViewportHeight = 1.f; // 100% of window height
    atlasView.setViewport(sf::FloatRect(0.f, 0.f, 0.25f, 1.f));
    float atlasAspectRatio = (windowWidth * atlasViewportWidth) / (windowHeight * atlasViewportHeight);
    atlasView.setSize(windowWidth * atlasViewportWidth, (windowWidth * atlasViewportWidth) / atlasAspectRatio);
    atlasView.setCenter(atlasView.getSize() / 2.f);
    atlasOriginalViewSize = atlasView.getSize(); // save the original view size for zoom functions

    // layer view initialization
    float layerViewportWidth = 0.75f; // 75% of window width
    float layerViewportHeight = 0.75f; // 75% of window height
    layerView.setViewport(sf::FloatRect(0.25f, 0.f, 0.75f, 0.75f));
    float layerAspectRatio = (windowWidth * layerViewportWidth) / (windowHeight * layerViewportHeight);
    layerView.setSize(windowWidth * layerViewportWidth, windowWidth * layerViewportWidth / layerAspectRatio);
    layerView.setCenter(layerView.getSize() / 2.f);
    layerOriginalViewSize = layerView.getSize();

    // vertical separator between the atlas and the layer/UI views
    verticalSeparator.setSize(sf::Vector2f(2.0f, static_cast<float>(window.getSize().y))); // 2px wide line
    verticalSeparator.setFillColor(sf::Color::White);
    verticalSeparator.setPosition(atlasView.getViewport().width * window.getSize().x, 0.0f);

    // horizontal separator between the layer view and the UI
    horizontalSeparator.setSize(sf::Vector2f(static_cast<float>(window.getSize().x), 2.0f)); // 2px tall line
    horizontalSeparator.setFillColor(sf::Color::White);
    horizontalSeparator.setPosition(atlasView.getViewport().width * window.getSize().x, layerView.getViewport().height * window.getSize().y);

    // initialize classes and structs
    InitializeClass();
}

void Editor::InitializeClass() {
    ui = new UI(*this); // create a new instance and pass a reference to the current Editor instance
    ui->Initialize();   // call specific initialization
    tileAtlas = new TileAtlas(*this);
    tileAtlas->Initialize();
    tileMap = new TileMap(*this, *tileAtlas);
}

void Editor::Run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();  // use deltatime to make actions relative to time not framerate
        HandleEvents(deltaTime);
        Render(window);
    }
}

void Editor::HandleEvents(float deltaTime) {
    sf::Event event;
    inputDelay -= deltaTime;

    // track mouse drag state for painting tiles, atlas selection and panning
    bool isLeftMouseDragging = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    bool isRightMouseDragging = sf::Mouse::isButtonPressed(sf::Mouse::Right);
    bool isMiddleMouseDragging = sf::Mouse::isButtonPressed(sf::Mouse::Middle);

    while (window.pollEvent(event)) {
        // get the mouse position within the window and convert it to world coordinates so we know which view the mouse was in when clicked
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePos);
        sf::Vector2f atlasMousePos = window.mapPixelToCoords(mousePos, atlasView);
        sf::Vector2f layerMousePos = window.mapPixelToCoords(mousePos, layerView);
        sf::Vector2f uiMousePos = window.mapPixelToCoords(mousePos, uiView);

        if (event.type == sf::Event::Closed) { window.close(); }

        int layerIndex = -1;    // default invalid index
        // key inputs to switch between the layers of a TileMap instance
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) { layerIndex = 0; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) { layerIndex = 1; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) { layerIndex = 2; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) { layerIndex = 3; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5)) { layerIndex = 4; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num6)) { layerIndex = 5; }
        // now set the current layer with layerIndex variable by calling SetCurrentLayer and passing it
        if (layerIndex != -1) { tileMap->SetCurrentLayer(layerIndex); }

        // ATLAS VIEW MOUSE INPUTS
        if (GetViewportBounds(atlasView, window).contains(static_cast<sf::Vector2f>(mousePos))) {
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {}
                if (event.mouseButton.button == sf::Mouse::Right) { tileAtlas->HandleSelection(atlasMousePos, true, deltaTime); }
                if (event.mouseButton.button == sf::Mouse::Middle) { tileAtlas->HandlePanning(atlasMousePos, true, deltaTime); }
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {}
                if (event.mouseButton.button == sf::Mouse::Right) { tileAtlas->HandleSelection(atlasMousePos, false, deltaTime); }
                if (event.mouseButton.button == sf::Mouse::Middle) { tileAtlas->HandlePanning(atlasMousePos, false, deltaTime); }
            }
            else if (event.type == sf::Event::MouseMoved) {
                if (isRightMouseDragging) { tileAtlas->HandleSelection(atlasMousePos, true, deltaTime); }
                if (isMiddleMouseDragging) { tileAtlas->HandlePanning(atlasMousePos, true, deltaTime); }
            }
            else if (event.type == sf::Event::MouseWheelMoved) {
                if (event.mouseWheel.delta > 0) { HandleAtlasZoom(atlasView, event.mouseWheel.delta, atlasOriginalViewSize); }  // zoom in
                else if (event.mouseWheel.delta < 0) { HandleAtlasZoom(atlasView, event.mouseWheel.delta, atlasOriginalViewSize); }  // zoom out
            }
        }

        // LAYER VIEW MOUSE INPUTS
        else if (GetViewportBounds(layerView, window).contains(static_cast<sf::Vector2f>(mousePos))) {
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left && !tileMap->showCollisionOverlay) { tileMap->HandleTilePlacement(layerMousePos); }
                else if (event.mouseButton.button == sf::Mouse::Left && tileMap->showCollisionOverlay) { tileMap->HandleCollisionPlacement(layerMousePos, true); }
                if (event.mouseButton.button == sf::Mouse::Right) { tileMap->HandleSelection(layerMousePos, true, deltaTime); }
                if (event.mouseButton.button == sf::Mouse::Middle) { tileMap->HandlePanning(layerMousePos, true, deltaTime); }
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {}
                if (event.mouseButton.button == sf::Mouse::Right) { tileMap->HandleSelection(layerMousePos, false, deltaTime); }
                if (event.mouseButton.button == sf::Mouse::Middle) { tileMap->HandlePanning(layerMousePos, false, deltaTime); }
            }
            else if (event.type == sf::Event::MouseMoved) {
                if (isLeftMouseDragging && !tileMap->showCollisionOverlay) { tileMap->HandleTilePlacement(layerMousePos); }
                else if (isLeftMouseDragging && tileMap->showCollisionOverlay) { tileMap->HandleCollisionPlacement(layerMousePos, true); }
                if (isRightMouseDragging) { tileMap->HandleSelection(layerMousePos, true, deltaTime); }
                if (isMiddleMouseDragging) { tileMap->HandlePanning(layerMousePos, true, deltaTime); }
            }
            else if (event.type == sf::Event::MouseWheelMoved) {
                if (event.mouseWheel.delta > 0) { HandleLayerZoom(layerView, event.mouseWheel.delta, layerOriginalViewSize); }  // zoom in
                else if (event.mouseWheel.delta < 0) { HandleLayerZoom(layerView, event.mouseWheel.delta, layerOriginalViewSize); }  // zoom out
            }
        }

        // UI VIEW MOUSE INPUTS
        else if (GetViewportBounds(uiView, window).contains(static_cast<sf::Vector2f>(mousePos))) {
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left && inputDelay <= 0.f) { ui->HandleInteraction(uiMousePos, window); }
                if (event.mouseButton.button == sf::Mouse::Right) {}
                if (event.mouseButton.button == sf::Mouse::Middle) {}
            }
            else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {}
                if (event.mouseButton.button == sf::Mouse::Right) {}
                if (event.mouseButton.button == sf::Mouse::Middle) {}
            }
        }

        ui->HandleTextInput(event);
        // reset the input delay
        inputDelay = 0.01f;
    }
}

void Editor::Render(sf::RenderWindow& window) {
    window.clear();

    // ui rendering
    window.setView(uiView);
    ui->DrawUI(window);
    ui->DrawTextInput(window);

    // layer rendering
    window.setView(layerView);
    if (tileMap->showMergedLayers) {
        tileMap->MergeAllLayers(window, true);
    }
    tileMap->DrawLayerGrid(window, tileMap->GetCurrentLayerIndex());
    tileMap->DrawDragSelection(window);
    if (tileMap->showCollisionOverlay) {
        tileMap->DrawCollisionOverlay(window, tileMap->GetCurrentLayerIndex());
    }

    // atlas rendering
    window.setView(atlasView);
    tileAtlas->DrawAtlas(window);
    tileAtlas->DrawDragSelection(window);

    // reset to default view for separators
    window.setView(window.getDefaultView());
    window.draw(verticalSeparator);
    window.draw(horizontalSeparator);

    // display to window
    window.display();
}

sf::FloatRect Editor::GetViewportBounds(const sf::View& view, const sf::RenderWindow& window) {
    // set the viewport rect to the passed in view's viewport
    sf::FloatRect viewport = view.getViewport();
    // determine and return the rectangle that represents the viewport's dimensions so we can determine a viewports bounds
    return sf::FloatRect(
        viewport.left * window.getSize().x,
        viewport.top * window.getSize().y,
        viewport.width * window.getSize().x,
        viewport.height * window.getSize().y
    );
}

void Editor::HandleAtlasZoom(sf::View& view, float delta, const sf::Vector2f& originalSize) {
    int newZoomIndex = currentZoomIndex + (delta < 0 ? -1 : 1); // set new zoom index based on if delta is positive or negative, if delta is negative, zoom out, and vice versa
    newZoomIndex = clamp(newZoomIndex, 0, static_cast<int>(zoomLevels.size()) - 1);
    if (newZoomIndex != currentZoomIndex) {
        currentZoomIndex = newZoomIndex;
        atlasScaleFactor = static_cast<float>(zoomLevels[currentZoomIndex]) / zoomLevels[0]; // Scale relative to the base
        tileAtlas->UpdateTileSize(atlasScaleFactor);
    }
}

void Editor::HandleLayerZoom(sf::View& view, float delta, const sf::Vector2f& originalSize) {
    int newZoomIndex = currentZoomIndex + (delta < 0 ? -1 : 1);
    newZoomIndex = clamp(newZoomIndex, 0, static_cast<int>(zoomLevels.size()) - 1);
    if (newZoomIndex != currentZoomIndex) {
        currentZoomIndex = newZoomIndex;
        layerScaleFactor = static_cast<float>(zoomLevels[currentZoomIndex]) / zoomLevels[0]; // scale relative to base level 
        tileMap->UpdateTileScale(layerScaleFactor);
    }
}
