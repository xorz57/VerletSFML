#pragma once

#include <SFML/Graphics/Color.hpp>

#include <glm/glm.hpp>

struct Object {
    Object(const glm::vec2 &position, float radius) : position(position),
                                                      position_last(position),
                                                      radius(radius) {}

    glm::vec2 position{0.0f, 0.0f};
    glm::vec2 position_last{0.0f, 0.0f};
    glm::vec2 acceleration{0.0f, 0.0f};
    float radius{0.0f};
    sf::Color color{sf::Color::White};
};
