#pragma once

#include "Object.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowStyle.hpp>

#include <vector>

class Application {
public:
    Application();
    void Run();

private:
    void ProcessEvents();
    void HandleEventClosed(const sf::Event &event);
    void HandleEventResized(const sf::Event &event);
    void HandleEventMouseWheelScrolled(const sf::Event &event);
    void HandleEventMouseButtonPressed(const sf::Event &event);
    void HandleEventMouseButtonReleased(const sf::Event &event);
    void FixedUpdate(const sf::Time &fixedDeltaTime);

    sf::VideoMode mMode{900u, 900u};
    sf::String mTitle{"VerletSFML"};
    sf::Uint32 mStyle{sf::Style::Default};
    sf::ContextSettings mSettings{24u, 8u, 8u, 3u, 3u};
    sf::RenderWindow mWindow;
    sf::View mView;
    std::vector<Object> mObjects;
    glm::vec2 mGravitationalAcceleration{0.0f, 1'000.0f};
    glm::vec2 constraintPosition{450.0f, 450.0f};
    float constraintRadius{450.0f};
    size_t mSelectedObjectIndex{0};
    bool mIsObjectSelected{false};
};
