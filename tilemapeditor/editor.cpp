#include "editor.h"
#include "ui.h"
#include "tilemap.h"
#include "tileatlas.h"

// default editor constructor because editor is the core manager
Editor::Editor()
    : window(sf::VideoMode(1920, 1080), "Tile Editor")
{
    auto windowWidth = static_cast<float>(window.getSize().x);
    auto windowHeight = static_cast<float>(window.getSize().y);

    // initialize default zoom level to match normal rendering
    float defaultZoomFactor = static_cast<float>(zoomLevels[currentZoomIndex])
        / zoomLevels[0];

    // ui view initialization
    float uiViewportWidth = 0.75f; // 75% of window width
    float uiViewportHeight = 0.25f; // 25% of window height
    uiView.setViewport(sf::FloatRect(0.25f, 0.75f, 0.75f, 0.25f));
    float uiAspectRatio = (windowWidth * uiViewportWidth) 
        / (windowHeight * uiViewportHeight);
    uiView.setSize(windowWidth * uiViewportWidth, 
        (windowWidth * uiViewportWidth) / uiAspectRatio);
    uiView.setCenter(uiView.getSize() / 2.f); // center the view

    // atlas view initialization
    float atlasViewportWidth = 0.25f; // 25% of window width
    float atlasViewportHeight = 1.f; // 100% of window height
    atlasView.setViewport(sf::FloatRect(0.f, 0.f, 0.25f, 1.f));
    float atlasAspectRatio = (windowWidth * atlasViewportWidth) / (
        windowHeight * atlasViewportHeight);
    atlasView.setSize(windowWidth * atlasViewportWidth, 
        (windowWidth * atlasViewportWidth) / atlasAspectRatio);
    atlasView.setCenter(atlasView.getSize() / 2.f);
    // save the original view size for zoom functions
    atlasOriginalViewSize = atlasView.getSize(); 

    // layer view initialization
    float layerViewportWidth = 0.75f; // 75% of window width
    float layerViewportHeight = 0.75f; // 75% of window height
    layerView.setViewport(sf::FloatRect(0.25f, 0.f, 0.75f, 0.75f));
    // set as
    float layerAspectRatio = (windowWidth * layerViewportWidth) 
        / (windowHeight * layerViewportHeight);
    layerView.setSize(windowWidth * layerViewportWidth, windowWidth
        * layerViewportWidth / layerAspectRatio);
    layerView.setCenter(layerView.getSize() / 2.f);
    // save the original view size for zoom functions
    layerOriginalViewSize = layerView.getSize();

    // vertical separator between all 3 views
    verticalSeparator.setSize(sf::Vector2f(2.0f, static_cast<float>(
        window.getSize().y))); // 2px wide line
    verticalSeparator.setFillColor(sf::Color::White);
    verticalSeparator.setPosition(atlasView.getViewport().width
        * window.getSize().x, 0.0f);

    // horizontal separator between layer and UI views
    horizontalSeparator.setSize(sf::Vector2f(static_cast<float>(
        window.getSize().x), 2.0f)); // 2px tall line
    horizontalSeparator.setFillColor(sf::Color::White);
    horizontalSeparator.setPosition(atlasView.getViewport().width
        * window.getSize().x, layerView.getViewport().height * window.getSize().y);

    // initialize classes and structs
    InitializeClass();
}

void Editor::InitializeClass() 
{
    // create a new instance and pass a reference to the current Editor instance
    ui = new UI(*this); 
    ui->Initialize();   
    tileAtlas = new TileAtlas(*this);
    tileAtlas->Initialize();
    tileMap = new TileMap(*this, *tileAtlas);
    // no tileMap initialization because it gets created upon ui interaction
}

void Editor::Run() 
{
    sf::Clock clock;
    while (window.isOpen()) {
        // use deltatime to make actions relative to time not framerate
        float deltaTime = clock.restart().asSeconds();  
        HandleEvents(deltaTime);
        Render(window);
    }
}

void Editor::HandleEvents(float deltaTime) 
{
    sf::Event event;
    inputDelay -= deltaTime;

    bool isLeftMouseDragging = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    bool isRightMouseDragging = sf::Mouse::isButtonPressed(sf::Mouse::Right);
    bool isMiddleMouseDragging = sf::Mouse::isButtonPressed(sf::Mouse::Middle);

    while (window.pollEvent(event)) {
        // Global events
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        ProcessKeyboardInputs();

        // Get the mouse position in different view coordinate systems
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f atlasMousePos = window.mapPixelToCoords(mousePos, atlasView);
        sf::Vector2f layerMousePos = window.mapPixelToCoords(mousePos, layerView);
        sf::Vector2f uiMousePos = window.mapPixelToCoords(mousePos, uiView);

        // delegate to view-specific handlers based on the viewport
        if (GetViewportBounds(atlasView, window).contains(static_cast
            <sf::Vector2f>(mousePos))) 
        {
            HandleAtlasEvents(event, atlasMousePos, deltaTime,
                isRightMouseDragging, isMiddleMouseDragging);
        }
        else if (GetViewportBounds(layerView, window).contains(
            static_cast<sf::Vector2f>(mousePos))) 
        {
            HandleLayerEvents(event, layerMousePos, deltaTime, 
                isLeftMouseDragging, isRightMouseDragging, isMiddleMouseDragging);
        }
        else if (GetViewportBounds(uiView, window).contains(static_cast<
            sf::Vector2f>(mousePos))) 
        {
            HandleUIEvents(event, uiMousePos, deltaTime);
        }

        // global text input handling for the UI
        ui->HandleTextInput(event);

        // reset input delay
        inputDelay = 0.01f;
    }
}

void Editor::ProcessKeyboardInputs() 
{
    int layerIndex = -1;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) { layerIndex = 0; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) { layerIndex = 1; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) { layerIndex = 2; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) { layerIndex = 3; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5)) { layerIndex = 4; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num6)) { layerIndex = 5; }
    if (layerIndex != -1) {
        tileMap->SetCurrentLayer(layerIndex);
    }
}

void Editor::HandleAtlasEvents(const sf::Event& event, 
    const sf::Vector2f& atlasMousePos, float deltaTime, 
    bool isRightDragging, bool isMiddleDragging) 
{
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Right)
            tileAtlas->HandleSelection(atlasMousePos, true, deltaTime);
        else if (event.mouseButton.button == sf::Mouse::Middle)
            tileAtlas->HandlePanning(atlasMousePos, true, deltaTime);
    }
    else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Right)
            tileAtlas->HandleSelection(atlasMousePos, false, deltaTime);
        else if (event.mouseButton.button == sf::Mouse::Middle)
            tileAtlas->HandlePanning(atlasMousePos, false, deltaTime);
    }
    else if (event.type == sf::Event::MouseMoved) {
        if (isRightDragging)
            tileAtlas->HandleSelection(atlasMousePos, true, deltaTime);
        if (isMiddleDragging)
            tileAtlas->HandlePanning(atlasMousePos, true, deltaTime);
    }
    else if (event.type == sf::Event::MouseWheelMoved) {
        // Zoom in or out depending on wheel delta
        HandleAtlasZoom(atlasView, event.mouseWheel.delta, atlasOriginalViewSize);
    }
}

void Editor::HandleLayerEvents(const sf::Event& event, 
    const sf::Vector2f& layerMousePos, float deltaTime, bool isLeftDragging,
    bool isRightDragging, bool isMiddleDragging) 
{
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (!tileMap->showCollisionOverlay)
                tileMap->HandleTilePlacement(layerMousePos);
            else
                tileMap->HandleCollisionPlacement(layerMousePos, true);
        }
        else if (event.mouseButton.button == sf::Mouse::Right) {
            tileMap->HandleSelection(layerMousePos, true, deltaTime);
        }
        else if (event.mouseButton.button == sf::Mouse::Middle) {
            tileMap->HandlePanning(layerMousePos, true, deltaTime);
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Right)
            tileMap->HandleSelection(layerMousePos, false, deltaTime);
        else if (event.mouseButton.button == sf::Mouse::Middle)
            tileMap->HandlePanning(layerMousePos, false, deltaTime);
    }
    else if (event.type == sf::Event::MouseMoved) {
        if (isLeftDragging) {
            if (!tileMap->showCollisionOverlay)
                tileMap->HandleTilePlacement(layerMousePos);
            else
                tileMap->HandleCollisionPlacement(layerMousePos, true);
        }
        if (isRightDragging)
            tileMap->HandleSelection(layerMousePos, true, deltaTime);
        if (isMiddleDragging)
            tileMap->HandlePanning(layerMousePos, true, deltaTime);
    }
    else if (event.type == sf::Event::MouseWheelMoved) {
        HandleLayerZoom(layerView, event.mouseWheel.delta, layerOriginalViewSize);
    }
}

void Editor::HandleUIEvents(const sf::Event& event, const sf::Vector2f& uiMousePos,
    float deltaTime) 
{
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left && inputDelay <= 0.f)
            ui->HandleInteraction(uiMousePos, window);
    }
}

void Editor::Render(sf::RenderWindow& window) 
{
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

sf::FloatRect Editor::GetViewportBounds(const sf::View& view, 
    const sf::RenderWindow& window) 
{
    sf::FloatRect viewport = view.getViewport();
    // determine and return the viewport rect dimensions to determine its bounds
    return sf::FloatRect(
        viewport.left * window.getSize().x,
        viewport.top * window.getSize().y,
        viewport.width * window.getSize().x,
        viewport.height * window.getSize().y
    );
}

void Editor::HandleAtlasZoom(sf::View& view, float delta, 
    const sf::Vector2f& originalSize) 
{
    // set new zoom index based on if scroll delta is positive or negative
    int newZoomIndex = currentZoomIndex + (delta < 0 ? -1 : 1); 
    newZoomIndex = clamp(newZoomIndex, 0, static_cast<int>(zoomLevels.size()) - 1);
    if (newZoomIndex != currentZoomIndex) {
        currentZoomIndex = newZoomIndex;
        // scale relative to the base zoom level
        atlasScaleFactor = static_cast<float>(zoomLevels[currentZoomIndex])
            / zoomLevels[0]; 
        tileAtlas->UpdateTileSize(atlasScaleFactor);
    }
}

void Editor::HandleLayerZoom(sf::View& view, float delta, 
    const sf::Vector2f& originalSize) 
{
    int newZoomIndex = currentZoomIndex + (delta < 0 ? -1 : 1);
    newZoomIndex = clamp(newZoomIndex, 0, static_cast<int>(zoomLevels.size()) - 1);
    if (newZoomIndex != currentZoomIndex) {
        currentZoomIndex = newZoomIndex;

        layerScaleFactor = static_cast<float>(zoomLevels[currentZoomIndex])
            / zoomLevels[0]; 
        tileMap->UpdateTileScale(layerScaleFactor);
    }
}