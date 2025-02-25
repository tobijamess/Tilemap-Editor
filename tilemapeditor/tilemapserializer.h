#ifndef TILEMAPSERIALIZER_H
#define TILEMAPSERIALIZER_H

#include "tilemap.h"
#include <iostream>

/*  object flow for saving and loading map data from files:
    tileData = object that holds an individual tiles properties like position, index etc.
    row = object that holds a full row of tileData objects
    tiles = object that holds multiple row objects that make up a full layer
    layerData = object that holds all data about a layer like dimensions, opacity, tiles etc.
    mapData = object that holds every layer
    tileData->row->tiles->layerData["tiles"]->mapData["layers"]
*/

bool TileMap::SaveTileMap(const std::string& filename) const
{
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

bool TileMap::LoadTileMap(const std::string& filename)
{
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
        newLayer.collisionGrid.resize(newLayer.height,
            std::vector<bool>(newLayer.width, false));
        const auto& collisionGridData = layerData["collisionGrid"];
        for (int y = 0; y < newLayer.height; ++y) {
            for (int x = 0; x < newLayer.width; ++x) {
                newLayer.collisionGrid[y][x] = collisionGridData[y][x];
            }
        }
        // push the new layer back into the vector of layers each iteration
        layers.push_back(newLayer);
    }
    activeLayerIndex = layers.empty() ? -1 : 0; // reset active layer
    // return true if loading succeeded
    return true;
}

#endif // !TILEMAPSERIALIZER_H
