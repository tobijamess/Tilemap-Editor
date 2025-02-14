#ifndef LAYER_H
#define LAYER_H

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <iostream>
#include <vector>
#include <set>
#include "json.hpp"
#include <fstream>

class Editor;
struct TileAtlas;

class TileMap {
private:
	Editor& editor;	// reference to Editor to avoid circular dependency
	TileAtlas& tileAtlas;

	struct Tile {
		sf::Sprite sprite;	// sprite for the individual tile
		int index = -1;  // index in the layer
		std::vector<sf::IntRect> textureRects; // all tiles' texture regions
		sf::Texture texture;    // texture of the tile
		sf::IntRect selectionBounds;    // drag selected area bounds
	};

	struct TileLayer {
		// controls the width and height of the layer
		int width;
		int height;
		bool isVisible;	// controls visibility of a entire layer, used for merging layers and hiding some specifically
		float opacity = 0.5f;	// controls the opacity of a layer, used during merge layers to make sure the active layer is opaque
		int index;	// the index of a tile layer, to access a layer specifically when they're combined into a game map
		std::vector<std::vector<Tile>> layer;	// 2D grid of tiles makes up an entire layer
		std::set<sf::Vector2i> selectedTiles;
		std::vector<std::vector<bool>> collisionGrid; // collision grid for the layer
	};

	bool isSelecting = false;
	sf::Vector2i selectionStartIndices; // drag-selection start
	sf::Vector2i selectionEndIndices;   // drag-selection end

	struct SelectedTile {
		std::vector<int> indices;  // index in the atlas
		std::vector<sf::IntRect> textureRects; // all selected tiles' texture regions
		sf::IntRect selectionBounds;    // drag selected area bounds
		sf::Sprite sprite;  // sprite created from texture and texture rect
	};

	std::vector<TileLayer> layers;	// vector to hold multiple layers
	int activeLayerIndex = -1;	// the index of the current active layer, defaulted to -1, used for setting the active/current layer based on index
	float layerTileSize = 16.0f;	// base tile size (e.g. 16x16)
	float layerScaleFactor = 1.0f;	// default scale factor for zooming

public:
	// public variables
	SelectedTile currentSelection;	// shared selection for both atlas and layer
	bool showMergedLayers = false;	// bool to decide whether to display merged layers or not
	bool showCollisionOverlay = false;	// bool to decide whether to display the collision overlay or not
	// main tileMap functions
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