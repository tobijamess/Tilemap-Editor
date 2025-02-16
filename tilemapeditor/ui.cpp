#include "ui.h"
#include "editor.h"
#include "tilemap.h"

UI::UI(Editor& editor) : editor(editor) {}

// load the ui font, will be called when a new ui instance is initialized in editor
bool UI::Initialize() 
{
    if (!font.loadFromFile("assets/fonts/font.ttf")) {
        std::cerr << "Failed to load font: \n";
        return false;
    }
    return true;
}

void UI::HandleInteraction(const sf::Vector2f& mousePos,
    sf::RenderWindow& window)
{
    for (const auto& button : buttons) {
        // check each buttons bounds to see if it contains the mouse position
        if (button.shape.getGlobalBounds().contains(mousePos)) {
            // get the label text from each button
            std::string label = button.label.getString();
            if (label == "Save Tilemap" || label == "Load Tilemap") {
                lastClickedButton = label;
                // activate text input for saving or loading a tile map file
                ActivateTextInput();    
            }
            // depending on which button was pressed, pass different layer sizes
            else if (label == "50x50 Grid") {
                editor.GetTileMap()->AddLayer(50, 50);
            }
            else if (label == "100x100 Grid") {
                editor.GetTileMap()->AddLayer(100, 100);
            }
            else if (label == "200x200 Grid") {
                editor.GetTileMap()->AddLayer(200, 200);
            }
            else if (label == "Merge Layers") {
                // toggle showMergedLayers bool everytime button is pressed
                editor.GetTileMap()->showMergedLayers 
                    = !editor.GetTileMap()->showMergedLayers; 
                // merge layers depending on the current bool state
                editor.GetTileMap()->MergeAllLayers(window,
                    editor.GetTileMap()->showMergedLayers); 
            }
            else if (label == "Toggle Collision") {
                editor.GetTileMap()->showCollisionOverlay
                    = !editor.GetTileMap()->showCollisionOverlay;
                editor.GetTileMap()->DrawCollisionOverlay(window,
                    editor.GetTileMap()->GetCurrentLayerIndex());
            }
            std::cout << "Button clicked: " << label << "\n";
            break; // exit once the click was handled
        }
    }
}

void UI::DrawUI(sf::RenderWindow& window) 
{
    // populate the buttons vector if empty 
    if (buttons.empty()) {
        // define the properties of the UI buttons
        sf::Vector2f buttonSize(200, 25); // button dimensions
        float buttonSpacing = 5.f;  // spacing between buttons
        // starting x, y position of buttons relative to the viewport
        float startX = 5.f;
        float startY = 5.f;
        // define the buttons labels
        std::vector<std::string> buttonLabels = {
            "50x50 Grid",
            "100x100 Grid",
            "200x200 Grid",
            "Merge Layers",
            "Toggle Collision",
            "Save Tilemap",
            "Load Tilemap"
        };
        // iterate through the button labels vector and create buttons
        for (size_t i = 0; i < buttonLabels.size(); ++i) {
            // create and position the buttons with the properties defined above
            Button button;
            button.shape.setSize(buttonSize);
            button.shape.setFillColor(sf::Color(150, 150, 150));
            button.shape.setPosition(startX, startY + i * (buttonSize.y
                + buttonSpacing));
            // set the properties of the button label
            button.label.setFont(font);
            button.label.setString(buttonLabels[i]);
            button.label.setCharacterSize(18);
            button.label.setFillColor(sf::Color::Black);
            // center the label on the button
            sf::FloatRect textBounds = button.label.getLocalBounds();
            button.label.setPosition(
                button.shape.getPosition().x + (buttonSize.x - textBounds.width)
                / 2.f - textBounds.left,
                button.shape.getPosition().y + (buttonSize.y - textBounds.height)
                / 2.f - textBounds.top
            );
            // store the button in the ui buttons vector
            buttons.push_back(button);
        }
    }
    for (const auto& button : buttons) {
        // draw the button and its label
        window.draw(button.shape);
        window.draw(button.label);
    }
}

void UI::ActivateTextInput() 
{
    isTextInputActive = true;
    inputText.clear();
    // set up input box
    inputBox.setSize(sf::Vector2f(300.f, 50.f));
    inputBox.setFillColor(sf::Color(200, 200, 200));
    inputBox.setPosition(240.f, 5.f);
    // set up input text display
    inputTextDisplay.setFont(font);
    inputTextDisplay.setCharacterSize(20);
    inputTextDisplay.setFillColor(sf::Color::Black);
    inputTextDisplay.setPosition(inputBox.getPosition().x + 10.f,
        inputBox.getPosition().y + 10.f);
}

void UI::HandleTextInput(const sf::Event& event)
{
    if (isTextInputActive) {
        if (event.type == sf::Event::TextEntered) {
            // handle text input (except special characters like backspace)
            if (event.text.unicode == '\b' && !inputText.empty()) {
                inputText.pop_back(); // handle Backspace
            }
            else if (event.text.unicode < 128 && event.text.unicode != '\b') {
                inputText += static_cast<char>(event.text.unicode);
            }
        }
        else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Enter) {
                // confirm input and deactivate text input mode
                isTextInputActive = false;
                std::cout << "Filename entered: " << inputText << "\n";
                // call save or load function
                if (lastClickedButton == "Save Tilemap") {
                    editor.GetTileMap()->SaveTileMap(inputText);
                }
                else if (lastClickedButton == "Load Tilemap") {
                    editor.GetTileMap()->LoadTileMap(inputText);
                }
            }
            else if (event.key.code == sf::Keyboard::Escape) {
                // cancel input
                isTextInputActive = false;
                inputText.clear();
            }
        }
    }
}

void UI::DrawTextInput(sf::RenderWindow& window)
{
    if (isTextInputActive) {
        inputTextDisplay.setString(inputText);
        window.draw(inputBox);
        window.draw(inputTextDisplay);
    }
}