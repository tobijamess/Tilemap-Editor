#include "layer.h"
#include "editor.h"
#include "tileatlas.h"

TileMap::TileMap(Editor& editor, TileAtlas& tileAtlas) : editor(editor), tileAtlas(tileAtlas) {}

void TileMap::Initialize(int width, int height) {

}

// -------------------------------------------------------------------------------------- TILE LAYER FUNCTIONS --------------------------------------------------------------------------------------

void TileMap::AddLayer(int width, int height) {
    // create a new TileLayer instance and set passed in properties
    TileLayer newLayer;
    newLayer.width = width;
    newLayer.height = height;
    newLayer.isVisible = true;
    newLayer.opacity = 1.0f;
    newLayer.index = layers.size();
    newLayer.layer.resize(height, std::vector<Tile>(width));
    newLayer.collisionGrid.resize(height, std::vector<bool>(width, false));
    layers.push_back(newLayer); // push the new layer back into the layers vector
    activeLayerIndex = layers.size() - 1;   // set this new layer as the current/active layer
}

void TileMap::AddTile(const sf::Texture& texture, const sf::IntRect& rect, int index, int x, int y) {
    if (activeLayerIndex < 0 || activeLayerIndex >= layers.size()) return;

    TileLayer& currentLayer = layers[activeLayerIndex];

    if (x >= 0 && x < currentLayer.width && y >= 0 && y < currentLayer.height) {
        Tile& tile = currentLayer.layer[y][x];
        tile.index = index;
        tile.sprite.setTexture(texture);
        tile.sprite.setTextureRect(rect);
        tile.sprite.setScale(layerScaleFactor, layerScaleFactor); // apply scale factor to the layer tiles
        tile.sprite.setPosition(x * layerTileSize, y * layerTileSize); // apply position based on the tile size
    }
}

void TileMap::RemoveTile(int x, int y) {

}

void TileMap::HandleTilePlacement(const sf::Vector2f& mousePos) {
    // if there is no texture selection in the currentSelection, exit early
    if (currentSelection.textureRects.empty()) return;

    // convert mouse position to grid coordinates, accounting for zooming and panning
    sf::Vector2f adjustedMousePos = (mousePos + editor.layerViewOffset) / layerScaleFactor;
    // grid position of the placement
    int gridX = static_cast<int>(adjustedMousePos.x / editor.baseTileSize);
    int gridY = static_cast<int>(adjustedMousePos.y / editor.baseTileSize);

    // iterate through all selected tiles
    for (size_t i = 0; i < currentSelection.textureRects.size(); ++i) {
        const auto& rect = currentSelection.textureRects[i];
        // calculate offset from the top-left corner of the selection
        int offsetX = (rect.left - currentSelection.selectionBounds.left) / editor.baseTileSize;
        int offsetY = (rect.top - currentSelection.selectionBounds.top) / editor.baseTileSize;
        // calculate tile's grid position on the map
        int targetX = gridX + offsetX;
        int targetY = gridY + offsetY;
        // determine tile index in the atlas
        int tileIndex = currentSelection.indices[i];
        // add the tile to the map
        AddTile(tileAtlas.GetTexture(), rect, tileIndex, targetX, targetY);
    }
}

void TileMap::DrawLayerGrid(sf::RenderTarget& target, int index) {
    if (index < 0 || index >= layers.size()) {  // don't try to draw the layer grid if a layer grid has not been created via the ui buttons
        std::cerr << "Invalid layer index for rendering: " << index << "\n";
        return;
    }
    sf::Vector2f offset = editor.layerViewOffset;   // get the offset of the layer view that is updated when panning
    const TileLayer& layer = layers[index]; // get the active TileLayer instance from the layers vector
// iterates over the tiles of the active layer, each tiles position is calculated based on its coordinates in the grid (x * layerTileSize, y * layerTileSize)
    for (int y = 0; y < layer.height; ++y) {
        for (int x = 0; x < layer.width; ++x) {
            sf::Vector2f tilePosition(static_cast<float>(x * layerTileSize), static_cast<float>(y * layerTileSize));
            const Tile& tile = layer.layer[y][x];
            if (tile.index >= 0) {
                sf::Sprite tileSprite = tile.sprite;    // get the sprite from the Tile struct
                tileSprite.setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(layer.opacity * 255)));
                tileSprite.setPosition(tilePosition - offset); // adjust the tile position with panning offset
                target.draw(tileSprite);    // draw the tile sprite in position
            }
        }
    }
    sf::RectangleShape line;    // create line shape to draw grid with
    line.setFillColor(sf::Color(100, 100, 100, 150));
    float startX = -offset.x;
    float startY = -offset.y;
    // iterate from the starting offset up to the grids width and height (layer width/height * layer tile size) and draw the grid lines
    for (float x = startX; x <= layer.width * layerTileSize - offset.x; x += layerTileSize) {
        line.setSize(sf::Vector2f(1.f, layer.height * layerTileSize)); // height of the grid
        line.setPosition(x, -offset.y); // position of each drawn line relative to the offset caused by panning
        target.draw(line);
    }
    // same here but for horizontal grid lines
    for (float y = startY; y <= layer.height * layerTileSize - offset.y; y += layerTileSize) {
        line.setSize(sf::Vector2f(layer.width * layerTileSize, 1.f)); // width of the grid
        line.setPosition(-offset.x, y);
        target.draw(line);
    }
}

// ------------------------------------------------------------------------------------ COLLISION LAYER FUNCTIONS ------------------------------------------------------------------------------------

void TileMap::HandleCollisionPlacement(const sf::Vector2f& mousePos, bool addCollision) {
    if (activeLayerIndex < 0 || activeLayerIndex >= layers.size()) return;

    TileLayer& currentLayer = layers[activeLayerIndex];
    sf::Vector2f adjustedMousePos = (mousePos + editor.layerViewOffset) / layerScaleFactor;

    int gridX = static_cast<int>(adjustedMousePos.x / editor.baseTileSize);
    int gridY = static_cast<int>(adjustedMousePos.y / editor.baseTileSize);

    if (gridX >= 0 && gridX < currentLayer.width &&
        gridY >= 0 && gridY < currentLayer.height) {
        currentLayer.collisionGrid[gridY][gridX] = addCollision;
    }
}

void TileMap::DrawCollisionOverlay(sf::RenderTarget& target, int index) {
    if (index < 0 || index >= layers.size()) return;

    const TileLayer& layer = layers[index];
    sf::RectangleShape collisionTile(sf::Vector2f(layerTileSize, layerTileSize));
    collisionTile.setFillColor(sf::Color(255, 0, 0, 100)); // Red semi-transparent

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

// --------------------------------------------------------------------------------------- SELECTION FUNCTIONS ---------------------------------------------------------------------------------------

void TileMap::HandleSelection(sf::Vector2f mousePos, bool isSelecting, float deltaTime) {
    sf::Vector2f adjustedMousePos = (mousePos + editor.layerViewOffset) / layerScaleFactor;
    sf::Vector2i gridPos(
        static_cast<int>(adjustedMousePos.x / editor.baseTileSize) * editor.baseTileSize,
        static_cast<int>(adjustedMousePos.y / editor.baseTileSize) * editor.baseTileSize
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

            // get selection bounds
            sf::IntRect bounds = GetSelectionBounds();

            // clear old selection
            currentSelection.textureRects.clear();
            currentSelection.indices.clear();

            // access the active layer
            if (activeLayerIndex >= 0 && activeLayerIndex < layers.size()) {
                TileLayer& currentLayer = layers[activeLayerIndex];

                // iterate over the selected grid area
                for (int y = bounds.top / editor.baseTileSize; y < (bounds.top + bounds.height) / editor.baseTileSize; ++y) {
                    for (int x = bounds.left / editor.baseTileSize; x < (bounds.left + bounds.width) / editor.baseTileSize; ++x) {
                        // ensure the coordinates are within the layer bounds
                        if (x >= 0 && x < currentLayer.width && y >= 0 && y < currentLayer.height) {
                            const Tile& tile = currentLayer.layer[y][x];

                            // add only if the tile is valid (has an index)
                            if (tile.index >= 0) {
                                currentSelection.textureRects.push_back(tile.sprite.getTextureRect());
                                currentSelection.indices.push_back(tile.index);
                            }
                        }
                    }
                }
            }
        }
    }
}


sf::IntRect TileMap::GetSelectionBounds() const {
    int left = std::min(selectionStartIndices.x, selectionEndIndices.x);
    int top = std::min(selectionStartIndices.y, selectionEndIndices.y);
    int right = std::max(selectionStartIndices.x, selectionEndIndices.x) + editor.baseTileSize;
    int bottom = std::max(selectionStartIndices.y, selectionEndIndices.y) + editor.baseTileSize;
    return sf::IntRect(left, top, right - left, bottom - top);
}

void TileMap::DrawDragSelection(sf::RenderTarget& target) {
    // when isSelecting is passed in as true to handle selection, draw a selection box based on the calculated bounds
    if (isSelecting) {
        sf::IntRect bounds = GetSelectionBounds();  // get the bounds of the selection rectangle based on the start and end selection indices
        // bounds left and top are scaled with atlasScaleFactor to reflect the zoom level, the atlasViewOffset is then subtracted to ensure alignment with a zoomed and panned grid
        sf::Vector2f adjustedPosition(
            (bounds.left * editor.layerScaleFactor) - editor.layerViewOffset.x,
            (bounds.top * editor.layerScaleFactor) - editor.layerViewOffset.y
        );
        // bounds width and height are also scaled with atlasScaleFactor to reflect the zoom level
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

// ---------------------------------------------------------------------------------------- UTILITY FUNCTIONS ----------------------------------------------------------------------------------------

void TileMap::SetCurrentLayer(int index) {
    if (index >= 0 && index < layers.size()) {
        activeLayerIndex = index;
        std::cout << "Switched to layer: " << activeLayerIndex << "\n";
    }
    else {
        std::cerr << "Invalid layer index: " << index << "\n";
    }
}

void TileMap::HandlePanning(sf::Vector2f mousePos, bool isPanning, float deltaTime) {
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
        lastMousePos = sf::Vector2f(0, 0);  // keep it at the current mouse position when panning stops
    }
}

void TileMap::UpdateTileScale(float scaleFactor) {
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

void TileMap::MergeAllLayers(sf::RenderTarget& target, bool showMergedLayers) {
    if (!showMergedLayers) return;  // if showMergedLayers was passed in as false, exit early
    sf::Vector2f offset = editor.layerViewOffset;   // get the offset from panning
    for (int i = 0; i < layers.size(); ++i) {   // loop through the layers vector drawing each layer with half opacity
        if (i == activeLayerIndex) continue;    // when the loop reaches the active layer, skip it as its already drawn
        const TileLayer& layer = layers[i]; // set layer variable to the current layer index the loop is at
        // if (!layer.isVisible) continue; // skip invisible layers
        // loop through the width and height of each current layer index drawing its' sprite tiles at half opacity
        for (int y = 0; y < layer.height; ++y) {
            for (int x = 0; x < layer.width; ++x) {
                const Tile& tile = layer.layer[y][x];   // get the tile at each width & height position from the current layer
                if (tile.index >= 0) {  // only draw valid tiles 
                    sf::Sprite tileSprite = tile.sprite;    // copy the tile sprite at this width and height of the layer being looped through
                    tileSprite.setColor(sf::Color(255, 255, 255, 100)); // set the sprite to half opacity
                    tileSprite.setPosition(
                        (x * layerTileSize) - offset.x,
                        (y * layerTileSize) - offset.y
                    );  // adjust the tile position relative to the offset
                    tileSprite.setScale(layerScaleFactor, layerScaleFactor);    // make sure it scales to the active layer's size if its' been zoomed
                    target.draw(tileSprite);
                }
            }
        }
    }
}

/*  object flow for saving and loading map data from files:
    tileData = object that holds an individual tiles properties like position, index etc.
    row = object that holds a full row of tileData objects
    tiles = object that holds multiple row objects that make up a full layer
    layerData = object that holds all data about a layer like dimensions, opacity, tiles etc.
    mapData = object that holds every layer
    tileData->row->tiles->layerData["tiles"]->mapData["layers"]
*/

bool TileMap::SaveTileMap(const std::string& filename) const {
    nlohmann::json mapData; // initialize json object to store the overall map data which consists of every layer (and their individual data)
    for (const auto& layer : layers) {  // iterate over all TileLayer objects (layer) in the layers vector
        nlohmann::json layerData;   // for each layer, a new json object called layerData is initialized to hold its data (dimensions, visiblity, opacity)
        layerData["width"] = layer.width;
        layerData["height"] = layer.height;
        layerData["isVisible"] = layer.isVisible;
        layerData["opacity"] = layer.opacity;
        // initialize json object to store rows of tiles from each layer
        nlohmann::json tiles;
        // for each row (y) in the layer, iterate through each tile (x) and construct a json representation for it
        for (int y = 0; y < layer.height; ++y) {
            nlohmann::json row; // initialize json object to store all tiles (tileData) that make up a row
            for (int x = 0; x < layer.width; ++x) {
                const Tile& tile = layer.layer[y][x];
                if (tile.index >= 0) {  // if the tile at layer[y][x] isn't empty, capture its properties and store in tileData json object
                    nlohmann::json tileData;
                    tileData["index"] = tile.index;
                    tileData["textureRect"] = {
                        {"left", tile.sprite.getTextureRect().left},
                        {"top", tile.sprite.getTextureRect().top},
                        {"width", tile.sprite.getTextureRect().width},
                        {"height", tile.sprite.getTextureRect().height}
                    };
                    tileData["position"] = {
                        {"x", tile.sprite.getPosition().x},
                        {"y", tile.sprite.getPosition().y}
                    };
                    row.push_back(tileData);    // push each the serialized tile into the row object
                }
                else {
                    row.push_back(nullptr); // else if the tile at layer[y][x] was empty, push back a nullptr to represent an empty tile
                }
            }
            tiles.push_back(row);   // push the entire row into the tiles object which holds all the tiles in a layer
        }
        layerData["tiles"] = tiles; // add the serialized tile data (tiles) into layerData["tiles"] array
        // serialize the collision grid data for each layer
        nlohmann::json collisionGridData;
        for (int y = 0; y < layer.height; ++y) {
            nlohmann::json row;
            for (int x = 0; x < layer.width; ++x) {
                row.push_back(layer.collisionGrid[y][x]); // add collision state
            }
            collisionGridData.push_back(row);
        }
        layerData["collisionGrid"] = collisionGridData;
        mapData["layers"].push_back(layerData); // then push all of this layer iterations data (layerData) into the mapData["layers"] array
    }
    // write the json data to a file which will be named whatever was typed during the ui interaction
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for saving: " << filename << "\n";
        return false;
    }
    file << mapData.dump(4); // pretty-print json with 4 space indentation for readability 
    return true;    // return true if saving succeeded
}

bool TileMap::LoadTileMap(const std::string& filename) {
    nlohmann::json mapData; // initialize a mapData object which will hold each l
    // open the file specified during the ui interaction
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for loading: " << filename << "\n";
        return false;
    }
    file >> mapData;    // parse the specified files contents into the mapData object
    layers.clear(); // clear any existing layers so there are no random layers visible when this map is loaded
    // iterate over each layer stored in the mapData["layers"] array
    for (const auto& layerData : mapData["layers"]) {
        TileLayer newLayer; // create new TileLayer object for each layer (which will be loaded and re-drawn) and populate it with the deserialized data
        newLayer.width = layerData["width"];
        newLayer.height = layerData["height"];
        newLayer.isVisible = layerData["isVisible"];
        newLayer.opacity = layerData["opacity"];
        newLayer.index = layers.size(); // set this new layer's index to match it's original index in the layers vector
        newLayer.layer.resize(newLayer.height, std::vector<Tile>(newLayer.width));  // resize the new layer grid (newLayer.layer) to its width and height
        // iterate through the "tiles" array from layerData and deserialize each tile
        const auto& tiles = layerData["tiles"];
        for (int y = 0; y < newLayer.height; ++y) {
            for (int x = 0; x < newLayer.width; ++x) {
                if (tiles[y][x].is_null()) continue; // skip empty tiles
                const auto& tileData = tiles[y][x]; // set the tileData for the [y][x] tile from the "tiles" array
                Tile& tile = newLayer.layer[y][x];  // create Tile struct object to hold the [y][x] tile from newLayer.layer (which is the new TileLayer struct's grid of tiles)
                // set the Tile struct object's attributes based on the deserialized json
                tile.index = tileData["index"];
                tile.sprite.setTexture(tileAtlas.GetTexture());
                tile.sprite.setTextureRect(sf::IntRect(
                    tileData["textureRect"]["left"],
                    tileData["textureRect"]["top"],
                    tileData["textureRect"]["width"],
                    tileData["textureRect"]["height"]
                ));
                tile.sprite.setPosition(
                    tileData["position"]["x"],
                    tileData["position"]["y"]
                );
                tile.sprite.setScale(layerScaleFactor, layerScaleFactor);
            }
        }
        newLayer.collisionGrid.resize(newLayer.height, std::vector<bool>(newLayer.width, false));
        const auto& collisionGridData = layerData["collisionGrid"];
        for (int y = 0; y < newLayer.height; ++y) {
            for (int x = 0; x < newLayer.width; ++x) {
                newLayer.collisionGrid[y][x] = collisionGridData[y][x];
            }
        }
        layers.push_back(newLayer); // push the new layer back into the vector of layers each iteration
    }
    activeLayerIndex = layers.empty() ? -1 : 0; // reset active layer
    return true;    // return true if loading succeeded
}