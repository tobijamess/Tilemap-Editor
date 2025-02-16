#ifndef LAYER_H
#define LAYER_H

#include <SFML/Graphics.hpp>
#include <set>
#include "json.hpp"
#include <fstream>

class Editor;
struct TileAtlas;

class TileMap {
private:
	// reference to Editor and TileAtlas to avoid circular dependency
	Editor& editor;			
	TileAtlas& tileAtlas;

	struct Tile {
		sf::Sprite sprite;						// sprite for the individual tile
		int index = -1;							// index in the layer
		std::vector<sf::IntRect> textureRects;	// all tiles' texture regions
		sf::Texture texture;					// texture of the tile
		sf::IntRect selectionBounds;			// drag selected area bounds
	};

	struct TileLayer {
		int width;				// controls the width and height of the layer
		int height;
		bool isVisible;			// used to hide inactive layers
		float opacity = 0.5f;	// used to change visiblity of active layer when merged
		int index;				// index to access specific layer in whole game map

		// 2D grid of tiles makes up an entire layer
		std::vector<std::vector<Tile>> layer;
		// ?????????????????????????????????????????
		// std::set<sf::Vector2i> selectedTiles;
		// collision grid for a specific layer
		std::vector<std::vector<bool>> collisionGrid;	
	};

	bool isSelecting = false;
	sf::Vector2i selectionStartIndices; // drag-selection start
	sf::Vector2i selectionEndIndices;   // drag-selection end

public:
	struct SelectedTileData {
		sf::IntRect textureRect;
		sf::Vector2i offset;			// relative grid offset from selection start 
	};

	struct SelectedTile {
		int index = -1;						// index in the atlas
		std::vector<SelectedTileData> tiles;	
		sf::IntRect selectionBounds;		// drag selected area bounds
		sf::Sprite sprite;					// sprite created from texture and texture rect
	};
private:
	std::vector<TileLayer> layers;	// vector to hold multiple layers
	int activeLayerIndex = -1;		// used for setting current active layer
	float layerTileSize = 16.0f;	// base tile size (e.g. 16x16)
	float layerScaleFactor = 1.0f;	// default scale factor for zooming

public:
	// shared selection for both atlas and layer
	SelectedTile currentSelection;		
	// bool to decide whether to display merged layers or not
	bool showMergedLayers = false;	
	// bool to decide whether to display the collision overlay or not
	bool showCollisionOverlay = false;	

	// main TileMap functions
	TileMap(Editor& editor, TileAtlas& tileAtlas);
	void Initialize(int width, int height);
	void DrawLayerGrid(sf::RenderTarget& target, int index);
	void SetCurrentLayer(int index);
	void AddTile(const sf::Texture& texture, const sf::IntRect& rect, int index, int x, int y);
	void RemoveTile(int x, int y);
	void HandleTilePlacement(const sf::Vector2f& mousePos);
	void HandlePanning(sf::Vector2f mousePos, bool isPanning, float deltaTime);
	void UpdateTileScale(float scaleFactor);
	void HandleSelection(sf::Vector2f mousePos, bool selecting, float deltaTime);
	sf::IntRect GetSelectionBounds() const;
	void DrawDragSelection(sf::RenderTarget& target);
	void ToggleVisibility();
	void ClearLayer();
	void AddLayer(int width, int height);
	void RemoveLayer(int index);
	void MergeAllLayers(sf::RenderTarget& target, bool showMergedLayers);
	void HandleCollisionPlacement(const sf::Vector2f& mousePos, bool addCollision);
	void AddCollisionTile(int gridX, int gridY);
	void DrawCollisionOverlay(sf::RenderTarget& target, int index);
	bool SaveTileMap(const std::string& filename) const;
	bool LoadTileMap(const std::string& filename);
	// getter functions
	const int GetTileSize() const { return layerTileSize; }
	int GetCurrentLayerIndex() { return activeLayerIndex; }
	std::vector<TileLayer>& GetLayers() { return layers; }
};
#endif