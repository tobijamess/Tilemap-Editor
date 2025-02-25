#ifndef TILEATLAS_H
#define TILEATLAS_H

#include "tilemap.h"

class Editor;

struct TileAtlas {
    Editor& editor;
    float deltaTime;                    // delta time for consistent timing
    float atlasTileSize = 16.0f;        // base tile size (e.g. 16x16)
    sf::Texture textureAtlas;           // atlas texture
    sf::Sprite atlasSprite;             // atlas sprite
    sf::Vector2f atlasPos = { 0, 0 };   // default atlas position

    bool isSelecting = false;
    sf::Vector2i selectionStartIndices; // drag-selection start
    sf::Vector2i selectionEndIndices;   // drag-selection end

    TileAtlas(Editor& editor);
    bool Initialize();
    void HandleSelection(sf::Vector2f mousePos, bool isDragging, float deltaTime);
    sf::IntRect GetSelectionBounds() const;
    void HandlePanning(sf::Vector2f mousePos, bool isPanning, float deltaTime);
    void UpdateTileSize(float scaleFactor);
    void DrawAtlas(sf::RenderTarget& target);
    void DrawDragSelection(sf::RenderTarget& target);
    // getter function to return information about the tile e.g. texture of a tile
    const sf::Texture& GetTexture() { return textureAtlas; }
};
#endif // !TILEATLAS_H