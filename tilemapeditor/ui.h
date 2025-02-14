#ifndef UI_H
#define UI_H

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <vector>
#include "editor.h"

class Editor;

class UI {
private:
    Editor& editor; // reference to Editor to avoid circular dependency
    sf::Font font;  // font variable for the text on buttons

    struct Button {
        sf::RectangleShape shape;   // rectangle for the actual button
        sf::Text label; // label of text that will sit on the button
    };
    std::vector<Button> buttons;    // list of all the button structs in the ui
    
    bool isTextInputActive = false; // tracks the text input for filenames (for save and load buttons)
    std::string inputText;  // store the input text (filename) entered
    sf::RectangleShape inputBox;    // rectangle element for the input box
    sf::Text inputTextDisplay;  // text to display the input to the screen
    std::string lastClickedButton;  // string to store which button was pressed (between save or load tilemap buttons)
public:
    UI(Editor& editor);
    bool Initialize();
    void HandleInteraction(const sf::Vector2f& mousePos, sf::RenderWindow& window);
    void ActivateTextInput();
    void HandleTextInput(const sf::Event& event);
    void DrawTextInput(sf::RenderWindow& window);
    void DrawUI(sf::RenderWindow& window);
};
#endif