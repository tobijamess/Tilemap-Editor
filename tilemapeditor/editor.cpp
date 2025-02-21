#include "editor.h"
#include "ui.h"
#include "tilemap.h"
#include "tileatlas.h"

// default editor constructor because editor is the core manager
Editor::Editor()
    : window(sf::VideoMode(1280, 720), "Tile Editor")
{
    settings.antialiasingLevel = 0;

    auto windowWidth = static_cast<float>(window.getSize().x);
    auto windowHeight = static_cast<float>(window.getSize().y);

    // initialize default zoom level to match normal rendering
    float defaultZoomFactor = static_cast<float>(zoomLevels[currentZoomIndex])
        / zoomLevels[0];

    InitializeUIView(uiView, window);
    InitializeAtlasView(atlasView, atlasOriginalViewSize, window);
    InitializeLayerView(layerView, layerOriginalViewSize, window);
    InitializeSeparatorView(separatorView, verticalSeparator,
        horizontalSeparator, window, atlasView, layerView);

    // initialize classes and structs
    InitializeClass();
}

void Editor::InitializeClass() 
{
    // create a new instance and pass a reference to the current Editor instance
    ui = std::make_shared<UI>(*this); 
    ui->Initialize();   
    tileAtlas = std::make_shared<TileAtlas>(*this);
    tileAtlas->Initialize();
    tileMap = std::make_shared<TileMap>(*this, *tileAtlas);
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

void Editor::HandleResize(const sf::Event& event) {
    // create event sizes vector to prevent conversion from event to vector2u errors
    sf::Vector2u newSize(event.size.width, event.size.height);

    // update the main window view to the new size
    sf::FloatRect visibleArea(0, 0, newSize.x, newSize.y);
    window.setView(sf::View(visibleArea));

    // reinitialize views with the updated window dimensions
    InitializeUIView(uiView, window);
    InitializeAtlasView(atlasView, atlasOriginalViewSize, window);
    InitializeLayerView(layerView, layerOriginalViewSize, window);
    InitializeSeparatorView(separatorView, verticalSeparator,
        horizontalSeparator, window, atlasView, layerView);

    // reset the UI layout so buttons are recreated in their new positions
    ui->ResetButtons();
}


void Editor::HandleEvents(float deltaTime) 
{
    sf::Event event;
    inputDelay -= deltaTime;

    bool isLeftMouseDragging = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    bool isRightMouseDragging = sf::Mouse::isButtonPressed(sf::Mouse::Right);
    bool isMiddleMouseDragging = sf::Mouse::isButtonPressed(sf::Mouse::Middle);

    while (window.pollEvent(event)) {
        // global events
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        else if (event.type == sf::Event::Resized) {
            HandleResize(event);
            continue;
        }

        ProcessKeyboardInputs();

        // get the mouse position in different view coordinate systems
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
        // zoom in or out depending on wheel delta
        HandleAtlasZoom(atlasView, event.mouseWheel.delta, atlasOriginalViewSize);
    }
}

void Editor::HandleLayerEvents(const sf::Event& event, 
    const sf::Vector2f& layerMousePos, float deltaTime, bool isLeftDragging,
    bool isRightDragging, bool isMiddleDragging) 
{
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (tileMap->eraserActive)
                tileMap->RemoveTile(layerMousePos);
            else if (!tileMap->showCollisionOverlay)
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
            if (tileMap->eraserActive)
            tileMap->RemoveTile(layerMousePos);
            else if (!tileMap->showCollisionOverlay)
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

    // separator rendering
    window.setView(separatorView);
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
    newZoomIndex = std::clamp(newZoomIndex, 0, static_cast<int>(zoomLevels.size()) - 1);
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
    newZoomIndex = std::clamp(newZoomIndex, 0, static_cast<int>(zoomLevels.size()) - 1);
    if (newZoomIndex != currentZoomIndex) {
        currentZoomIndex = newZoomIndex;

        layerScaleFactor = static_cast<float>(zoomLevels[currentZoomIndex])
            / zoomLevels[0]; 
        tileMap->UpdateTileScale(layerScaleFactor);
    }
}