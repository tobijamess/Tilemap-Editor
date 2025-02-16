#include "tileatlas.h"
#include "editor.h"

TileAtlas::TileAtlas(Editor& editor) : editor(editor) {}

// function to load the image into a texture which will be used as the tile atlas, tileWidth/Height will be defined as 16, 16 when called in the editor
bool TileAtlas::Initialize()
{
    if (!textureAtlas.loadFromFile("assets/map/tilemap16.png")) { return false; }
    atlasSprite.setTexture(textureAtlas);
    atlasSprite.setPosition(0.f, 0.f); // set to top left of the atlas viewport
    return true;
}

void TileAtlas::HandleSelection(sf::Vector2f mousePos, bool isSelecting, float deltaTime) 
{
    // Adjust mouse position by accounting for panning and zoom
    sf::Vector2f adjustedMousePos = (mousePos + editor.atlasViewOffset) / editor.atlasScaleFactor;

    // Snap to grid using the base tile size
    sf::Vector2i texturePos(
        static_cast<int>(adjustedMousePos.x / editor.baseTileSize) * editor.baseTileSize,
        static_cast<int>(adjustedMousePos.y / editor.baseTileSize) * editor.baseTileSize
    );

    if (isSelecting) {
        if (!this->isSelecting) {
            this->isSelecting = true;
            selectionStartIndices = texturePos;
            editor.GetTileMap()->currentSelection.selectionBounds.left = texturePos.x;
            editor.GetTileMap()->currentSelection.selectionBounds.top = texturePos.y;
        }
        // update selection bounds as the mouse moves
        selectionEndIndices = texturePos;
        editor.GetTileMap()->currentSelection.selectionBounds.width = texturePos.x
            - editor.GetTileMap()->currentSelection.selectionBounds.left;
        editor.GetTileMap()->currentSelection.selectionBounds.height = texturePos.y
            - editor.GetTileMap()->currentSelection.selectionBounds.top;
    }
    else {
        if (this->isSelecting) {
            this->isSelecting = false;
            // Finalize the selection bounds
            sf::IntRect bounds = GetSelectionBounds();
            editor.GetTileMap()->currentSelection.selectionBounds = bounds;
            // Clear any previous selection data
            editor.GetTileMap()->currentSelection.tiles.clear();

            // Compute the starting tile indices in grid units
            int startTileX = selectionStartIndices.x / editor.baseTileSize;
            int startTileY = selectionStartIndices.y / editor.baseTileSize;

            // Loop over the selected region and record each tile with its relative offset
            for (int y = bounds.top; y < bounds.top + bounds.height; y += editor.baseTileSize) {
                for (int x = bounds.left; x < bounds.left + bounds.width; x += editor.baseTileSize) {
                    TileMap::SelectedTileData data;
                    data.textureRect = sf::IntRect(x, y, editor.baseTileSize, editor.baseTileSize);
                    int currentTileX = x / editor.baseTileSize;
                    int currentTileY = y / editor.baseTileSize;
                    data.offset = sf::Vector2i(currentTileX - startTileX, currentTileY - startTileY);
                    editor.GetTileMap()->currentSelection.tiles.push_back(data);
                }
            }
        }
    }
}

void TileAtlas::DrawAtlas(sf::RenderTarget& target)
{
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

void TileAtlas::DrawDragSelection(sf::RenderTarget& target)
{
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

sf::IntRect TileAtlas::GetSelectionBounds() const 
{
    int left = std::min(selectionStartIndices.x, selectionEndIndices.x);
    int top = std::min(selectionStartIndices.y, selectionEndIndices.y);
    int right = std::max(selectionStartIndices.x, selectionEndIndices.x) + editor.baseTileSize;
    int bottom = std::max(selectionStartIndices.y, selectionEndIndices.y) + editor.baseTileSize;
    return sf::IntRect(left, top, right - left, bottom - top);  // return the selection bounds
}

void TileAtlas::HandlePanning(sf::Vector2f mousePos, bool isPanning,
    float deltaTime) 
{
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

void TileAtlas::UpdateTileSize(float scaleFactor) 
{
    // calculate new tile size for zooming using the base tile size and scale factor
    atlasTileSize = static_cast<int>(editor.baseTileSize * scaleFactor);
}