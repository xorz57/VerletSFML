#include "number_generator.hpp"

#include <SFML/Graphics.hpp>

#include <glm/gtc/constants.hpp>

#include <cmath>
#include <vector>

struct VerletObject
{
    VerletObject(sf::Vector2f position_, float radius_) : position{position_}, position_last{position_}, radius{radius_} {}

    void update(float dt)
    {
        const sf::Vector2f displacement = position - position_last;
        position_last = position;
        position = position + displacement + acceleration * (dt * dt);
        acceleration = {};
    }

    void setVelocity(sf::Vector2f v, float dt)
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
    uint32_t mSubSteps = 1;
    sf::Vector2f mGravitationalAcceleration{0.0f, 1000.0f};
    sf::Vector2f mConstraintCenter{0.0f, 0.0f};
    float mConstraintRadius = 100.0f;
    std::vector<VerletObject> mVerletObjects;
    float mTime = 0.0f;
    float mFrameDt = 0.0f;

    mConstraintCenter = {0.5f * 1000.0f, 0.5f * 1000.0f};
    mConstraintRadius = 450.0f;
    mSubSteps = 8;
    mFrameDt = 1.0f / static_cast<float>(60u);

    const float object_spawn_delay = 0.025f;
    const float object_spawn_speed = 1200.0f;
    const sf::Vector2f object_spawn_position = {500.0f, 200.0f};
    const float object_min_radius = 1.0f;
    const float object_max_radius = 20.0f;
    const uint32_t max_objects_count = 1000;
    const float max_angle = 1.0f;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 1;

    sf::RenderWindow window(sf::VideoMode(1000u, 1000u), "VerletSFML", sf::Style::Default, settings);
    window.setFramerateLimit(60u);

    sf::Clock clock;

    while (window.isOpen())
    {
        ProcessEvents(window);

        if (mVerletObjects.size() < max_objects_count && clock.getElapsedTime().asSeconds() >= object_spawn_delay)
        {
            clock.restart();
            auto &object = mVerletObjects.emplace_back(object_spawn_position, RNGf::getRange(object_min_radius, object_max_radius));
            const float t = mTime;
            const float angle = max_angle * sin(t) + glm::pi<float>() * 0.5f;
            object.setVelocity(object_spawn_speed * sf::Vector2f{cos(angle), sin(angle)}, mFrameDt / static_cast<float>(mSubSteps));
            object.color = GetRainbow(t);
        }

        mTime += mFrameDt;
        const float step_dt = mFrameDt / static_cast<float>(mSubSteps);
        for (uint32_t i{mSubSteps}; i--;)
        {
            for (auto &verletObject : mVerletObjects)
            {
                verletObject.acceleration += mGravitationalAcceleration;
            }
            const float response_coef = 0.75f;

            for (size_t i = 0; i < mVerletObjects.size(); ++i)
            {
                VerletObject &object_1 = mVerletObjects[i];
                for (size_t k = i + 1; k < mVerletObjects.size(); ++k)
                {
                    VerletObject &object_2 = mVerletObjects[k];
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
            for (auto &verletObject : mVerletObjects)
            {
                const sf::Vector2f v = mConstraintCenter - verletObject.position;
                const float dist = sqrt(v.x * v.x + v.y * v.y);
                if (dist > (mConstraintRadius - verletObject.radius))
                {
                    const sf::Vector2f n = v / dist;
                    verletObject.position = mConstraintCenter - n * (mConstraintRadius - verletObject.radius);
                }
            }
            for (auto &verletObject : mVerletObjects)
            {
                verletObject.update(step_dt);
            }
        }

        window.clear(sf::Color::White);

        const sf::Vector3f constraint{mConstraintCenter.x, mConstraintCenter.y, mConstraintRadius};
        sf::CircleShape constraint_background{constraint.z};
        constraint_background.setOrigin(constraint.z, constraint.z);
        constraint_background.setFillColor(sf::Color::Black);
        constraint_background.setPosition(constraint.x, constraint.y);
        constraint_background.setPointCount(128);
        window.draw(constraint_background);

        sf::CircleShape circle{1.0f};
        circle.setPointCount(32);
        circle.setOrigin(1.0f, 1.0f);
        for (const auto &verletObject : mVerletObjects)
        {
            circle.setPosition(verletObject.position);
            circle.setScale(verletObject.radius, verletObject.radius);
            circle.setFillColor(verletObject.color);
            window.draw(circle);
        }

        window.display();
    }

    return 0;
}
