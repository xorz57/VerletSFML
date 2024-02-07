#include <SFML/Graphics.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <random>
#include <vector>

struct Object {
    Object(const sf::Vector2f &position, float radius) : position(position),
                                                         position_last(position),
                                                         radius(radius) {}

    sf::Vector2f position{0.0f, 0.0f};
    sf::Vector2f position_last{0.0f, 0.0f};
    sf::Vector2f acceleration{0.0f, 0.0f};
    float radius{0.0f};
    sf::Color color{sf::Color::White};
};

static sf::Color GetRainbow(float t) {
    constexpr float c1 = 0.0f;
    constexpr float c2 = 2.0f * glm::pi<float>() / 3.0f;
    constexpr float c3 = 4.0f * glm::pi<float>() / 3.0f;
    const float f1 = glm::sin(t + c1);
    const float f2 = glm::sin(t + c2);
    const float f3 = glm::sin(t + c3);
    const auto r = static_cast<sf::Uint8>(255.0f * f1 * f1);
    const auto g = static_cast<sf::Uint8>(255.0f * f2 * f2);
    const auto b = static_cast<sf::Uint8>(255.0f * f3 * f3);
    return {r, g, b};
}

void HandleEventClosed(sf::Window &window, const sf::Event &) {
    window.close();
}

void HandleEventKeyPressed(sf::Window &window, const sf::Event &event) {
    if (event.key.code == sf::Keyboard::Escape) {
        window.close();
        return;
    }
}

void ProcessEvents(sf::Window &window) {
    sf::Event event{};
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                HandleEventClosed(window, event);
                break;
            case sf::Event::KeyPressed:
                HandleEventKeyPressed(window, event);
                break;
            default:
                break;
        }
    }
}

int main() {
    sf::Vector2f gravitationalAcceleration(0.0f, 1'000.0f);
    std::vector<Object> objects;

    sf::Vector2f constraintCenter(450.0f, 450.0f);
    float constraintRadius = 450.0f;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(1.0, 20.0);

    sf::VideoMode mode(900u, 900u);
    sf::String title("VerletSFML");
    sf::ContextSettings settings{24u, 8u, 8u, 3u, 3u};
    sf::RenderWindow window(mode, title, sf::Style::Default, settings);
    window.setFramerateLimit(60u);

    float deltaTime = 1.0f / 60.0f;
    float totalTime = 0.0f;

    uint32_t steps = 8u;

    sf::Clock clock;

    while (window.isOpen()) {
        ProcessEvents(window);

        totalTime += deltaTime;
        const float stepDeltaTime = deltaTime / static_cast<float>(steps);

        if (objects.size() < 1'000 && clock.getElapsedTime().asSeconds() >= 0.025f) {
            clock.restart();

            Object object(sf::Vector2f(450.0f, 50.0f), dis(gen));
            const float angle = 1.0f * glm::sin(totalTime) + 0.5f * glm::pi<float>();
            const sf::Vector2f velocity = 1'200.0f * sf::Vector2f(glm::cos(angle), glm::sin(angle));
            object.position_last = object.position - (velocity * stepDeltaTime);
            object.color = GetRainbow(totalTime);

            objects.push_back(object);
        }

        for (uint32_t step = 0; step < steps; step++) {
            for (auto &object: objects) {
                object.acceleration += gravitationalAcceleration;
            }

            for (size_t i = 0; i < objects.size(); ++i) {
                Object &object1 = objects[i];

                for (size_t j = i + 1; j < objects.size(); ++j) {
                    Object &object2 = objects[j];

                    const sf::Vector2f dPosition = object1.position - object2.position;
                    const float dPositionLength = glm::sqrt(dPosition.x * dPosition.x + dPosition.y * dPosition.y);
                    const sf::Vector2f dPositionNormalized = dPosition / dPositionLength;

                    const float minDistance = object1.radius + object2.radius;
                    if (dPositionLength < minDistance) {
                        const float responseCoefficient = 0.75f;
                        const float delta = 0.5f * responseCoefficient * (dPositionLength - minDistance);
                        const float massRatio1 = object1.radius / (object1.radius + object2.radius);
                        const float massRatio2 = object2.radius / (object1.radius + object2.radius);
                        object1.position -= dPositionNormalized * (massRatio2 * delta);
                        object2.position += dPositionNormalized * (massRatio1 * delta);
                    }
                }
            }

            for (auto &object: objects) {
                const sf::Vector2f dPosition = constraintCenter - object.position;
                const float dPositionLength = glm::sqrt(dPosition.x * dPosition.x + dPosition.y * dPosition.y);
                const sf::Vector2f dPositionNormalized = dPosition / dPositionLength;

                const float maxDistance = constraintRadius - object.radius;
                if (dPositionLength > maxDistance) {
                    object.position = constraintCenter - maxDistance * dPositionNormalized;
                }
            }

            for (auto &object: objects) {
                const sf::Vector2f displacement = object.position - object.position_last;
                object.position_last = object.position;
                object.position = object.position + displacement + object.acceleration * (stepDeltaTime * stepDeltaTime);
                object.acceleration = {};
            }
        }

        window.clear(sf::Color(25, 25, 25));

        sf::CircleShape circle1;
        circle1.setPointCount(128u);
        circle1.setRadius(constraintRadius);
        circle1.setOrigin(constraintRadius, constraintRadius);
        circle1.setPosition(constraintCenter);
        circle1.setFillColor(sf::Color::Black);
        window.draw(circle1);

        for (const Object &object: objects) {
            sf::CircleShape circle2;
            circle2.setPointCount(32u);
            circle2.setRadius(object.radius);
            circle2.setOrigin(object.radius, object.radius);
            circle2.setPosition(object.position);
            circle2.setFillColor(object.color);
            window.draw(circle2);
        }

        window.display();
    }

    return 0;
}
