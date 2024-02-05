#include "number_generator.hpp"

#include <SFML/Graphics.hpp>

#include <glm/gtc/constants.hpp>

#include <cmath>
#include <vector>

struct Object
{
    Object(const sf::Vector2f &position, float radius) : position(position),
                                                         position_last(position),
                                                         radius(radius) {}

    void Update(float dt)
    {
        const sf::Vector2f displacement = position - position_last;
        position_last = position;
        position = position + displacement + acceleration * (dt * dt);
        acceleration = {};
    }

    void SetVelocity(sf::Vector2f v, float dt)
    {
        position_last = position - (v * dt);
    }

    sf::Vector2f position{0.0f, 0.0f};
    sf::Vector2f position_last{0.0f, 0.0f};
    sf::Vector2f acceleration{0.0f, 0.0f};
    float radius = 10.0f;
    sf::Color color = sf::Color::White;
};

static sf::Color GetRainbow(float t)
{
    const float r = sin(t);
    const float g = sin(t + 0.33f * 2.0f * glm::pi<float>());
    const float b = sin(t + 0.66f * 2.0f * glm::pi<float>());
    return {
        static_cast<uint8_t>(255.0f * r * r),
        static_cast<uint8_t>(255.0f * g * g),
        static_cast<uint8_t>(255.0f * b * b)};
}

void HandleEventClosed(sf::Window &window, const sf::Event &)
{
    window.close();
}

void HandleEventKeyPressed(sf::Window &window, const sf::Event &event)
{
    if (event.key.code == sf::Keyboard::Escape)
    {
        window.close();
        return;
    }
}

void ProcessEvents(sf::Window &window)
{
    sf::Event event{};
    while (window.pollEvent(event))
    {
        switch (event.type)
        {
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

int main()
{
    sf::Vector2f gravitationalAcceleration{0.0f, 1000.0f};
    std::vector<Object> objects;
    float time = 0.0f;

    sf::Vector2f constraintCenter = {0.5f * 1000.0f, 0.5f * 1000.0f};
    float constraintRadius = 450.0f;
    uint32_t subSteps = 8;
    float frameDeltaTime = 1.0f / 60.0f;

    const float objectSpawnDelay = 0.025f;
    const float objectSpawnSpeed = 1200.0f;
    const sf::Vector2f objectSpawnPosition = {500.0f, 200.0f};
    const float objectMinRadius = 1.0f;
    const float objectMaxRadius = 20.0f;
    const uint32_t maxObjectsCount = 1000;
    const float objectSpawnMaxAngle = 1.0f;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;

    sf::RenderWindow window(sf::VideoMode(1000u, 1000u), "VerletSFML", sf::Style::Default, settings);
    window.setFramerateLimit(60u);

    sf::Clock clock;

    while (window.isOpen())
    {
        ProcessEvents(window);

        if (objects.size() < maxObjectsCount && clock.getElapsedTime().asSeconds() >= objectSpawnDelay)
        {
            clock.restart();
            auto &object = objects.emplace_back(objectSpawnPosition, RNGf::getRange(objectMinRadius, objectMaxRadius));
            const float t = time;
            const float angle = objectSpawnMaxAngle * sin(t) + glm::pi<float>() * 0.5f;
            object.SetVelocity(objectSpawnSpeed * sf::Vector2f{cos(angle), sin(angle)}, frameDeltaTime / static_cast<float>(subSteps));
            object.color = GetRainbow(t);
        }

        time += frameDeltaTime;
        const float step_dt = frameDeltaTime / static_cast<float>(subSteps);
        for (uint32_t i{subSteps}; i--;)
        {
            for (auto &object : objects)
            {
                object.acceleration += gravitationalAcceleration;
            }
            const float response_coef = 0.75f;

            for (size_t i = 0; i < objects.size(); ++i)
            {
                Object &object_1 = objects[i];
                for (size_t k = i + 1; k < objects.size(); ++k)
                {
                    Object &object_2 = objects[k];
                    const sf::Vector2f v = object_1.position - object_2.position;
                    const float dist2 = v.x * v.x + v.y * v.y;
                    const float min_dist = object_1.radius + object_2.radius;
                    if (dist2 < min_dist * min_dist)
                    {
                        const float dist = sqrt(dist2);
                        const sf::Vector2f n = v / dist;
                        const float mass_ratio_1 = object_1.radius / (object_1.radius + object_2.radius);
                        const float mass_ratio_2 = object_2.radius / (object_1.radius + object_2.radius);
                        const float delta = 0.5f * response_coef * (dist - min_dist);
                        object_1.position -= n * (mass_ratio_2 * delta);
                        object_2.position += n * (mass_ratio_1 * delta);
                    }
                }
            }
            for (auto &object : objects)
            {
                const sf::Vector2f v = constraintCenter - object.position;
                const float dist = sqrt(v.x * v.x + v.y * v.y);
                if (dist > (constraintRadius - object.radius))
                {
                    const sf::Vector2f n = v / dist;
                    object.position = constraintCenter - n * (constraintRadius - object.radius);
                }
            }
            for (auto &object : objects)
            {
                object.Update(step_dt);
            }
        }

        window.clear(sf::Color::White);

        const sf::Vector3f constraint{constraintCenter.x, constraintCenter.y, constraintRadius};
        sf::CircleShape constraint_background{constraint.z};
        constraint_background.setOrigin(constraint.z, constraint.z);
        constraint_background.setFillColor(sf::Color::Black);
        constraint_background.setPosition(constraint.x, constraint.y);
        constraint_background.setPointCount(128);
        window.draw(constraint_background);

        sf::CircleShape circle;
        circle.setRadius(1.0f);
        circle.setPointCount(32u);
        circle.setOrigin(1.0f, 1.0f);
        for (const auto &object : objects)
        {
            circle.setPosition(object.position);
            circle.setScale(object.radius, object.radius);
            circle.setFillColor(object.color);
            window.draw(circle);
        }

        window.display();
    }

    return 0;
}
