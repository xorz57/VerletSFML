#include <SFML/Graphics.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <random>
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
    const float c1 = glm::sin(t);
    const float c2 = glm::sin(t + 0.33f * 2.0f * glm::pi<float>());
    const float c3 = glm::sin(t + 0.66f * 2.0f * glm::pi<float>());
    const auto r = static_cast<sf::Uint8>(255.0f * c1 * c1);
    const auto g = static_cast<sf::Uint8>(255.0f * c2 * c2);
    const auto b = static_cast<sf::Uint8>(255.0f * c3 * c3);
    return sf::Color(r, g, b);
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
    sf::Vector2f gravitationalAcceleration(0.0f, 1'000.0f);
    std::vector<Object> objects;

    sf::Vector2f constraintCenter(500.0f, 500.0f);
    float constraintRadius = 450.0f;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(1.0, 20.0);

    sf::ContextSettings settings{24u, 8u, 8u, 3u, 3u};
    sf::RenderWindow window(sf::VideoMode(1'000u, 1'000u), "VerletSFML", sf::Style::Default, settings);
    window.setFramerateLimit(60u);

    float frameDeltaTime = 1.0f / 60.0f;
    float totalTime = 0.0f;

    uint32_t steps = 8u;

    sf::Clock clock;

    while (window.isOpen())
    {
        ProcessEvents(window);

        totalTime += frameDeltaTime;
        const float stepDeltaTime = frameDeltaTime / static_cast<float>(steps);

        if (objects.size() < 1'000 && clock.getElapsedTime().asSeconds() >= 0.025f)
        {
            clock.restart();
            Object object(sf::Vector2f(500.0f, 200.0f), dis(gen));
            const float angle = 1.0f * glm::sin(totalTime) + 0.5f * glm::pi<float>();
            object.SetVelocity(1'200.0f * sf::Vector2f(glm::cos(angle), glm::sin(angle)), stepDeltaTime);
            object.color = GetRainbow(totalTime);
            objects.push_back(object);
        }

        for (uint32_t step = steps; step--;)
        {
            for (auto &object : objects)
            {
                object.acceleration += gravitationalAcceleration;
            }
            const float responseCoefficient = 0.75f;

            for (size_t i = 0; i < objects.size(); ++i)
            {
                Object &object1 = objects[i];
                for (size_t k = i + 1; k < objects.size(); ++k)
                {
                    Object &object2 = objects[k];

                    const sf::Vector2f dPosition = object1.position - object2.position;
                    const float dPositionLength = glm::sqrt(dPosition.x * dPosition.x + dPosition.y * dPosition.y);
                    const sf::Vector2f dPositionNormalized = dPosition / dPositionLength;

                    const float minDistance = object1.radius + object2.radius;
                    if (dPositionLength < minDistance)
                    {
                        const float massRatio1 = object1.radius / (object1.radius + object2.radius);
                        const float massRatio2 = object2.radius / (object1.radius + object2.radius);
                        const float delta = 0.5f * responseCoefficient * (dPositionLength - minDistance);
                        object1.position -= dPositionNormalized * (massRatio2 * delta);
                        object2.position += dPositionNormalized * (massRatio1 * delta);
                    }
                }
            }
            for (auto &object : objects)
            {
                const sf::Vector2f dPosition = constraintCenter - object.position;
                const float dPositionLength = glm::sqrt(dPosition.x * dPosition.x + dPosition.y * dPosition.y);
                const sf::Vector2f dPositionNormalized = dPosition / dPositionLength;

                const float maxDistance = constraintRadius - object.radius;
                if (dPositionLength > maxDistance)
                {
                    object.position = constraintCenter - maxDistance * dPositionNormalized;
                }
            }
            for (auto &object : objects)
            {
                object.Update(stepDeltaTime);
            }
        }

        window.clear(sf::Color(25, 25, 25));

        // Draw Constraint
        sf::CircleShape circle1;
        circle1.setPointCount(128u);
        circle1.setRadius(constraintRadius);
        circle1.setOrigin(constraintRadius, constraintRadius);
        circle1.setPosition(constraintCenter);
        circle1.setFillColor(sf::Color::Black);
        window.draw(circle1);

        // Draw Objects
        for (const Object &object : objects)
        {
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
