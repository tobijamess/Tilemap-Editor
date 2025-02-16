#include "tilemap.h"
#include "editor.h"
#include "tileatlas.h"

TileMap::TileMap(Editor& editor, TileAtlas& tileAtlas) 
    : editor(editor), tileAtlas(tileAtlas) {}

// -------------------------------- TILE LAYER FUNCTIONS --------------------------------

void TileMap::AddLayer(int width, int height) 
{
    // create a new TileLayer instance and set passed in properties
    TileLayer newLayer;
    newLayer.width = width;
    newLayer.height = height;
    newLayer.isVisible = true;
    newLayer.opacity = 1.0f;
    newLayer.index = layers.size();
    newLayer.layer.resize(height, std::vector<Tile>(width));
    newLayer.collisionGrid.resize(height, std::vector<bool>(width, false));
    // push the new layer back into the layers vector
    layers.push_back(newLayer);
    // set this new layer as the current / active layer
    activeLayerIndex = layers.size() - 1;
}

void TileMap::AddTile(const sf::Texture& texture, const sf::IntRect& rect,
    int index, int x, int y) 
{
    if (activeLayerIndex < 0 || activeLayerIndex >= layers.size()) return;

    TileLayer& currentLayer = layers[activeLayerIndex];

    if (x >= 0 && x < currentLayer.width && y >= 0 && y < currentLayer.height) {
        Tile& tile = currentLayer.layer[y][x];
        tile.index = index;
        tile.sprite.setTexture(texture);
        tile.sprite.setTextureRect(rect);
        // apply scale factor to the layer tiles
        tile.sprite.setScale(layerScaleFactor, layerScaleFactor); 
        // apply position based on the tile size
        tile.sprite.setPosition(x * layerTileSize, y * layerTileSize);  
    }
}

void TileMap::RemoveTile(int x, int y) 
{

}

void TileMap::HandleTilePlacement(const sf::Vector2f& mousePos)
{
    // if there is no texture selection in the currentSelection, exit early
    if (currentSelection.tiles.empty()) return;

    // convert mouse position to grid coordinates (accounting for zoom/panning)
    sf::Vector2f adjustedMousePos = (mousePos + editor.layerViewOffset)
        / layerScaleFactor;
    int gridX = static_cast<int>(adjustedMousePos.x / editor.baseTileSize);
    int gridY = static_cast<int>(adjustedMousePos.y / editor.baseTileSize);

    // iterate through each selected tile
    for (const auto& tileData : currentSelection.tiles) {
        // compute the target grid position using the stored offset
        int targetX = gridX + tileData.offset.x;
        int targetY = gridY + tileData.offset.y;
        // optionally update currentSelection's index based on tile’s atlas position
        currentSelection.index = (tileData.textureRect.top / editor.baseTileSize) *
            (tileAtlas.GetTexture().getSize().x / editor.baseTileSize) +
            (tileData.textureRect.left / editor.baseTileSize);
        // place the tile on the current layer
        AddTile(tileAtlas.GetTexture(), tileData.textureRect, currentSelection.index,
            targetX, targetY);
    }
}

void TileMap::DrawLayerGrid(sf::RenderTarget& target, int index)
{
    // don't try to draw a non-existant layer to the window
    if (index < 0 || index >= layers.size()) {  
        std::cerr << "Invalid layer index for rendering: " << index << "\n";
        return;
    }

    // get the offset of the layer view that is updated when panning
    sf::Vector2f offset = editor.layerViewOffset;
    // get the active TileLayer instance from the layers vector
    const TileLayer& layer = layers[index]; 

    // each tiles position is calculated based on its coordinates in the grid
    // (x * layerTileSize, y * layerTileSize)
    for (int y = 0; y < layer.height; ++y) {
        for (int x = 0; x < layer.width; ++x) {
            sf::Vector2f tilePosition(static_cast<float>(x * layerTileSize),
                static_cast<float>(y * layerTileSize));
            const Tile& tile = layer.layer[y][x];
            // if tile exists, retrieve it from the Tile struct, set position and draw it
            if (tile.index >= 0) {
                sf::Sprite tileSprite = tile.sprite;      
                tileSprite.setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(
                    layer.opacity * 255)));
                // adjust position relative to panning offset
                tileSprite.setPosition(tilePosition - offset);
                target.draw(tileSprite);
            }
        }
    }

    // line shape for drawing grids
    sf::RectangleShape line;
    line.setFillColor(sf::Color(100, 100, 100, 150));
    float startX = -offset.x;
    float startY = -offset.y;

    // iterate from the starting offset up to the grids width and height and draw it
    for (float x = startX; x <= layer.width * layerTileSize - offset.x; x += layerTileSize) {
        // height of the grid
        line.setSize(sf::Vector2f(1.f, layer.height * layerTileSize));
        // position of each drawn line relative to panning offset
        line.setPosition(x, -offset.y); 
        target.draw(line);
    }
    // same here but for horizontal grid lines
    for (float y = startY; y <= layer.height * layerTileSize - offset.y; y += layerTileSize) {
        // width of the grid
        line.setSize(sf::Vector2f(layer.width * layerTileSize, 1.f));
        line.setPosition(-offset.x, y);
        target.draw(line);
    }
}

// -------------------------------- COLLISION LAYER FUNCTIONS --------------------------------

void TileMap::HandleCollisionPlacement(const sf::Vector2f& mousePos,
    bool addCollision)
{
    if (activeLayerIndex < 0 || activeLayerIndex >= layers.size()) return;

    TileLayer& currentLayer = layers[activeLayerIndex];
    sf::Vector2f adjustedMousePos = (mousePos + editor.layerViewOffset)
        / layerScaleFactor;

    int gridX = static_cast<int>(adjustedMousePos.x / editor.baseTileSize);
    int gridY = static_cast<int>(adjustedMousePos.y / editor.baseTileSize);

    if (gridX >= 0 && gridX < currentLayer.width &&
        gridY >= 0 && gridY < currentLayer.height) {
        currentLayer.collisionGrid[gridY][gridX] = addCollision;
    }
}

void TileMap::DrawCollisionOverlay(sf::RenderTarget& target, int index) 
{
    if (index < 0 || index >= layers.size()) return;

    const TileLayer& layer = layers[index];
    sf::RectangleShape collisionTile(sf::Vector2f(layerTileSize, layerTileSize));
    collisionTile.setFillColor(sf::Color(255, 0, 0, 100)); // semi-transparent red

    for (int y = 0; y < layer.height; ++y) {
        for (int x = 0; x < layer.width; ++x) {
            if (layer.collisionGrid[y][x]) {
                collisionTile.setPosition(
                    x * layerTileSize - editor.layerViewOffset.x,
                    y * layerTileSize - editor.layerViewOffset.y
                );
                target.draw(collisionTile);
            }
        }
    }
}

// -------------------------------- SELECTION FUNCTIONS --------------------------------

void TileMap::HandleSelection(sf::Vector2f mousePos, bool isSelecting,
    float deltaTime)
{
    sf::Vector2f adjustedMousePos = (mousePos + editor.layerViewOffset)
        / layerScaleFactor;

    sf::Vector2i gridPos(
        static_cast<int>(adjustedMousePos.x / editor.baseTileSize)
        * editor.baseTileSize,
        static_cast<int>(adjustedMousePos.y / editor.baseTileSize)
        * editor.baseTileSize
    );

    if (isSelecting) {
        if (!this->isSelecting) {
            this->isSelecting = true;
            selectionStartIndices = gridPos; // store the starting grid position
        }
        selectionEndIndices = gridPos; // continuously update the end position
    }
    else {
        if (this->isSelecting) {
            this->isSelecting = false;

            // get selection bounds and store in currentSelection
            sf::IntRect bounds = GetSelectionBounds();
            currentSelection.selectionBounds = bounds;

            // clear previous selection
            currentSelection.tiles.clear();

            // access the active layer
            if (activeLayerIndex >= 0 && activeLayerIndex < layers.size()) {
                TileLayer& currentLayer = layers[activeLayerIndex];

                // compute starting tile indices (in grid units)
                int startTileX = selectionStartIndices.x / editor.baseTileSize;
                int startTileY = selectionStartIndices.y / editor.baseTileSize;

                // iterate over the selected grid area (tile indices)
                for (int ty = bounds.top / editor.baseTileSize; ty < (bounds.top 
                    + bounds.height) / editor.baseTileSize; ++ty) {
                    for (int tx = bounds.left / editor.baseTileSize; tx < (bounds.left
                        + bounds.width) / editor.baseTileSize; ++tx) {
                        // ensure the coordinates are within the layer bounds
                        if (tx >= 0 && tx < currentLayer.width && ty >= 0 
                            && ty < currentLayer.height) {
                            const Tile& tile = currentLayer.layer[ty][tx];

                            // only add valid tiles (non-empty)
                            if (tile.index >= 0) {
                                SelectedTileData data;
                                data.textureRect = tile.sprite.getTextureRect();
                                // calculate offset relative to selection start
                                data.offset = sf::Vector2i(tx - startTileX, 
                                    ty - startTileY);
                                currentSelection.tiles.push_back(data);
                            }
                        }
                    }
                }
            }
        }
    }
}


sf::IntRect TileMap::GetSelectionBounds() const 
{
    int left = std::min(selectionStartIndices.x, selectionEndIndices.x);
    int top = std::min(selectionStartIndices.y, selectionEndIndices.y);
    int right = std::max(selectionStartIndices.x, selectionEndIndices.x)
        + editor.baseTileSize;
    int bottom = std::max(selectionStartIndices.y, selectionEndIndices.y)
        + editor.baseTileSize;
    return sf::IntRect(left, top, right - left, bottom - top);
}

void TileMap::DrawDragSelection(sf::RenderTarget& target) 
{
    // when isSelecting is true, draw selection box
    if (isSelecting) {
        // get the bounds of selection rectangle based on selection indices
        sf::IntRect bounds = GetSelectionBounds();  
        // bounds left/top are scaled with atlasScaleFactor to reflect the zoom level,
        // the atlasViewOffset is then subtracted to align with a zoomed/panned grid
        sf::Vector2f adjustedPosition(
            (bounds.left * editor.layerScaleFactor) - editor.layerViewOffset.x,
            (bounds.top * editor.layerScaleFactor) - editor.layerViewOffset.y
        );
        // bounds width/height are scaled with atlasScaleFactor to reflect zoom level
        sf::Vector2f adjustedSize(
            bounds.width * editor.layerScaleFactor,
            bounds.height * editor.layerScaleFactor
        );
        sf::RectangleShape selectionRect(adjustedSize);
        selectionRect.setPosition(adjustedPosition);
        selectionRect.setFillColor(sf::Color(0, 255, 0, 100));
        target.draw(selectionRect);
    }
}

// -------------------------------- UTILITY FUNCTIONS --------------------------------
void TileMap::SetCurrentLayer(int index) 
{
    if (index >= 0 && index < layers.size()) {
        activeLayerIndex = index;
        std::cout << "Switched to layer: " << activeLayerIndex << "\n";
    }
    else {
        std::cerr << "Invalid layer index: " << index << "\n";
    }
}

void TileMap::HandlePanning(sf::Vector2f mousePos, bool isPanning, float deltaTime)
{
    static sf::Vector2f lastMousePos;
    if (isPanning) {
        if (lastMousePos != sf::Vector2f(0, 0)) {
            sf::Vector2f delta = lastMousePos - mousePos;
            editor.GetLayerView().move(delta);  // update camera/view position
            editor.layerViewOffset += delta;    // update the grid offset

        }
        lastMousePos = mousePos;    // update last mouse position each frame
    }
    else {
        // keep it at the current mouse position when panning stops
        lastMousePos = sf::Vector2f(0, 0); 
    }
}

void TileMap::UpdateTileScale(float scaleFactor) 
{
    layerScaleFactor = scaleFactor;
    layerTileSize = editor.baseTileSize * layerScaleFactor;

    for (TileLayer& layer : layers) {
        for (int y = 0; y < layer.height; ++y) {
            for (int x = 0; x < layer.width; ++x) {
                Tile& tile = layer.layer[y][x];
                if (tile.index >= 0) { // update only valid tiles
                    tile.sprite.setScale(layerScaleFactor, layerScaleFactor);
                    tile.sprite.setPosition(x * layerTileSize, y * layerTileSize);
                }
            }
        }
    }
}

void TileMap::MergeAllLayers(sf::RenderTarget& target, bool showMergedLayers) 
{
    // if showMergedLayers was passed in as false, exit early
    if (!showMergedLayers) return;  
    sf::Vector2f offset = editor.layerViewOffset;   // get the offset from panning
    // loop through layers drawing them at 0.5 opacity
    for (int i = 0; i < layers.size(); ++i) {
        // when the loop reaches the active layer, skip it as its already drawn
        if (i == activeLayerIndex) continue;    
        // set layer variable to the current layer index the loop is at
        const TileLayer& layer = layers[i];
        // skip invisible layers
        // if (!layer.isVisible) continue; 
        // loop through the width/height of current layer drawing tiles at 0.5 opacity
        for (int y = 0; y < layer.height; ++y) {
            for (int x = 0; x < layer.width; ++x) {
                const Tile& tile = layer.layer[y][x];
                // only draw valid tiles 
                if (tile.index >= 0) {
                    // copy the tile at this width/height of the current layer
                    sf::Sprite tileSprite = tile.sprite;
                    // set the tile sprite to 0.5 opacity
                    tileSprite.setColor(sf::Color(255, 255, 255, 100)); 
                    tileSprite.setPosition(
                        (x * layerTileSize) - offset.x,
                        (y * layerTileSize) - offset.y
                    );  
                    // make sure it scales to the active layer's size if its' been zoomed
                    tileSprite.setScale(layerScaleFactor, layerScaleFactor);    
                    target.draw(tileSprite);
                }
            }
        }
    }
}