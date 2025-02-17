#ifndef UTILITY_H
#define UTILITY_H

namespace Utility { 
    // snaps mouse position to tg grid defined by viewOffset, scaleFactor, and baseTileSize.
    inline sf::Vector2i SnapToGrid(const sf::Vector2f& mousePos, 
        const sf::Vector2f& viewOffset, float scaleFactor, float baseTileSize)
    { 
        // adjust the mouse position for current views panning and zoom. 
        sf::Vector2f adjustedPos = (mousePos + viewOffset) / scaleFactor;
        // calculate snapped grid pos by flooring to nearest multiple of baseTileSize.
        int gridX = static_cast<int>(adjustedPos.x / baseTileSize) 
            * static_cast<int>(baseTileSize); 
        int gridY = static_cast<int>(adjustedPos.y / baseTileSize) 
            * static_cast<int>(baseTileSize); 
        return { gridX, gridY };
    } 
}

#endif // !UTILITY_H