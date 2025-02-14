#include "tileatlas.h"
#include "editor.h"

TileAtlas::TileAtlas(Editor& editor) : editor(editor) {}

// function to load the image into a texture which will be used as the tile atlas, tileWidth/Height will be defined as 16, 16 when called in the editor
bool TileAtlas::Initialize() {
    if (!textureAtlas.loadFromFile("assets/map/tilemap16.png")) { return false; }
    atlasSprite.setTexture(textureAtlas);
    atlasSprite.setPosition(0.f, 0.f); // set to top left of the atlas viewport
    return true;
}

void TileAtlas::HandleSelection(sf::Vector2f mousePos, bool isSelecting, float deltaTime) {
    // adjust mouse position by adding the atlas view offset and dividing by the scale factor
    sf::Vector2f adjustedMousePos = (mousePos + editor.atlasViewOffset) / editor.atlasScaleFactor;
    // snap the mouse selection to the nearest grid position (so selection start and end are always in a gridcell)
    sf::Vector2i texturePos(
        static_cast<int>(adjustedMousePos.x / editor.baseTileSize) * editor.baseTileSize,
        static_cast<int>(adjustedMousePos.y / editor.baseTileSize) * editor.baseTileSize
    );
    // if true was passed in from handle events, and we aren't already selecting, start a new selection and store the starting indices
    if (isSelecting) {
        if (!this->isSelecting) {
            this->isSelecting = true;   // start a new selection
            selectionStartIndices = texturePos; // store the starting grid position
            editor.GetTileMap()->currentSelection.selectionBounds.left = texturePos.x;
            editor.GetTileMap()->currentSelection.selectionBounds.top = texturePos.y;
        }
        selectionEndIndices = texturePos;   // update the end of the selection constantly until false is passed in
        editor.GetTileMap()->currentSelection.selectionBounds.width = texturePos.x - editor.GetTileMap()->currentSelection.selectionBounds.left;
        editor.GetTileMap()->currentSelection.selectionBounds.height = texturePos.y - editor.GetTileMap()->currentSelection.selectionBounds.top;
    }
    else {  // else if false was passed in (right click released), finalize the selection and get the whole selections bounds
        if (this->isSelecting) {
            this->isSelecting = false;  // finalize the selection
            // calculate selection bounds
            sf::IntRect bounds = GetSelectionBounds();
            editor.GetTileMap()->currentSelection.selectionBounds = bounds;
            // clear previous selections and populate textureRects with the new selection for placement
            editor.GetTileMap()->currentSelection.textureRects.clear();
            editor.GetTileMap()->currentSelection.indices.clear();

            sf::Vector2u atlasSize = textureAtlas.getSize();
            int tilesPerRow = atlasSize.x / editor.baseTileSize;
            for (int y = bounds.top; y < bounds.top + bounds.height; y += editor.baseTileSize) {
                for (int x = bounds.left; x < bounds.left + bounds.width; x += editor.baseTileSize) {
                    editor.GetTileMap()->currentSelection.textureRects.emplace_back(x, y, editor.baseTileSize, editor.baseTileSize);
                    int tileIndex = (y / editor.baseTileSize) * tilesPerRow + (editor.baseTileSize);
                    editor.GetTileMap()->currentSelection.indices.push_back(tileIndex);
                }
            }
        }
    }
}

void TileAtlas::DrawAtlas(sf::RenderTarget& target) {
    sf::Vector2f offset = editor.atlasViewOffset;   // offset is based on the view offset which updates when panning
    float scaledTileSize = atlasTileSize; // scaledTileSize is based on tileSize which updates when zooming
    // scale the atlas sprite tiles based on the zoom
    atlasSprite.setScale(scaledTileSize / editor.baseTileSize, scaledTileSize / editor.baseTileSize);
    atlasSprite.setPosition(-offset);   // set the atlas sprite position based on the panning offset
    target.draw(atlasSprite);
    // draw grid with fixed dimensions of 50x100
    int gridWidth = 50;
    int gridHeight = 100;
    sf::RectangleShape line;    // create line shape to draw grid with
    line.setFillColor(sf::Color(100, 100, 100, 150));
    // starting positions of horizontal and vertical gridlines based on the offset 
    float startX = -offset.x;
    float startY = -offset.y;
    // iterate from the starting offset up to the grids width and height (gridWidth/Height * atlasTileSize) and draw the grid lines
    for (float x = startX; x <= gridWidth * atlasTileSize - offset.x; x += atlasTileSize) {
        line.setSize(sf::Vector2f(1.f, gridHeight * atlasTileSize)); // height of the grid
        line.setPosition(x, -offset.y); // position of each drawn line relative to the offset caused by panning
        target.draw(line);
    }
    // same here but for horizontal grid lines
    for (float y = startY; y <= gridHeight * atlasTileSize - offset.y; y += atlasTileSize) {
        line.setSize(sf::Vector2f(gridWidth * atlasTileSize, 1.f)); // width of the grid
        line.setPosition(-offset.x, y);
        target.draw(line);
    }
}

void TileAtlas::DrawDragSelection(sf::RenderTarget& target) {
    // when isSelecting is passed in as true to handle selection, draw a selection box based on the calculated bounds
    if (isSelecting) {
        sf::IntRect bounds = GetSelectionBounds();  // get the bounds of the selection rectangle based on the start and end selection indices
        // bounds left and top are scaled with atlasScaleFactor to reflect the zoom level, the atlasViewOffset is then subtracted to ensure alignment with a zoomed and panned grid
        sf::Vector2f adjustedPosition(
            (bounds.left * editor.atlasScaleFactor) - editor.atlasViewOffset.x,
            (bounds.top * editor.atlasScaleFactor) - editor.atlasViewOffset.y
        );
        // bounds width and height are also scaled with atlasScaleFactor to reflect the zoom level
        sf::Vector2f adjustedSize(
            bounds.width * editor.atlasScaleFactor,
            bounds.height * editor.atlasScaleFactor
        );
        sf::RectangleShape selectionRect(adjustedSize);
        selectionRect.setPosition(adjustedPosition);
        selectionRect.setFillColor(sf::Color(0, 255, 0, 100));
        target.draw(selectionRect);
    }
}

sf::IntRect TileAtlas::GetSelectionBounds() const {
    int left = std::min(selectionStartIndices.x, selectionEndIndices.x);
    int top = std::min(selectionStartIndices.y, selectionEndIndices.y);
    int right = std::max(selectionStartIndices.x, selectionEndIndices.x) + editor.baseTileSize;
    int bottom = std::max(selectionStartIndices.y, selectionEndIndices.y) + editor.baseTileSize;
    return sf::IntRect(left, top, right - left, bottom - top);  // return the selection bounds
}

void TileAtlas::HandlePanning(sf::Vector2f mousePos, bool isPanning, float deltaTime) {
    static sf::Vector2f lastMousePos;
    if (isPanning) {
        if (lastMousePos != sf::Vector2f(0, 0)) {
            sf::Vector2f delta = lastMousePos - mousePos;
            editor.GetAtlasView().move(delta);
            editor.atlasViewOffset += delta;
        }
        lastMousePos = mousePos;
    }
    else {
        lastMousePos = sf::Vector2f(0, 0);
    }
}

void TileAtlas::UpdateTileSize(float scaleFactor) {
    // calculate new tile size for zooming using the base tile size and scale factor
    atlasTileSize = static_cast<int>(editor.baseTileSize * scaleFactor);
}