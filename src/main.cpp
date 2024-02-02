#include <SFML/Graphics.hpp>

#include "number_generator.hpp"
#include "renderer.hpp"
#include "solver.hpp"

#include <glm/gtc/constants.hpp>

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
    Solver solver;
    solver.setConstraint({0.5f * 1000.0f, 0.5f * 1000.0f}, 450.0f);
    solver.setSubStepsCount(8);
    solver.setSimulationUpdateRate(60u);

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

    Renderer renderer{window};

    sf::Clock clock;

    while (window.isOpen())
    {
        ProcessEvents(window);

        if (solver.getObjectsCount() < max_objects_count && clock.getElapsedTime().asSeconds() >= object_spawn_delay)
        {
            clock.restart();
            auto &object = solver.addObject(object_spawn_position, RNGf::getRange(object_min_radius, object_max_radius));
            const float t = solver.getTime();
            const float angle = max_angle * sin(t) + glm::pi<float>() * 0.5f;
            solver.setObjectVelocity(object, object_spawn_speed * sf::Vector2f{cos(angle), sin(angle)});
            object.color = GetRainbow(t);
        }

        solver.update();

        window.clear(sf::Color::White);
        renderer.render(solver);
        window.display();
    }

    return 0;
}
