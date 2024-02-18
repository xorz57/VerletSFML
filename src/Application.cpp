#include "Application.hpp"
#include "Object.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Event.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui-SFML.h>
#include <imgui.h>

#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <random>

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

Application::Application() {
    mWindow.create(mMode, mTitle, mStyle, mSettings);
    mWindow.setFramerateLimit(60u);
    mView = mWindow.getDefaultView();
}

void Application::Run() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(1.0, 20.0);

    (void) ImGui::SFML::Init(mWindow);

    const sf::Time fixedDeltaTime = sf::seconds(1.0f / 480.0f);
    sf::Time totalTime = sf::Time::Zero;
    sf::Time accumulator = sf::Time::Zero;

    sf::Clock deltaClock;
    sf::Clock spawnClock;

    while (mWindow.isOpen()) {
        ProcessEvents();

        const sf::Time deltaTime = deltaClock.restart();
        accumulator += deltaTime;
        while (accumulator > fixedDeltaTime) {
            FixedUpdate(fixedDeltaTime);
            totalTime += fixedDeltaTime;
            accumulator -= fixedDeltaTime;
        }

        if (mIsObjectSelected) {
            const sf::Vector2i pixelPosition = sf::Mouse::getPosition(mWindow);
            const sf::Vector2f worldPosition = mWindow.mapPixelToCoords(pixelPosition);
            Object &selectedObject = mObjects[mSelectedObjectIndex];
            selectedObject.position = glm::vec2(worldPosition.x, worldPosition.y);
            const glm::vec2 dPosition = constraintPosition - selectedObject.position;
            if (const float dRadius = constraintRadius - selectedObject.radius; glm::length(dPosition) > dRadius) {
                selectedObject.position = constraintPosition - dRadius * glm::normalize(dPosition);
            }
            selectedObject.position_last = selectedObject.position;
        }

        if (mObjects.size() < 1'000 && spawnClock.getElapsedTime().asSeconds() >= 0.025f) {
            spawnClock.restart();

            Object object(glm::vec2(450.0f, 50.0f), dis(gen));
            const float angle = 1.0f * glm::sin(totalTime.asSeconds()) + 0.5f * glm::pi<float>();
            const glm::vec2 velocity = 1'200.0f * glm::vec2(glm::cos(angle), glm::sin(angle));
            object.position_last = object.position - (velocity * fixedDeltaTime.asSeconds());
            object.color = GetRainbow(totalTime.asSeconds());

            mObjects.push_back(object);
        }

        ImGui::SFML::Update(mWindow, deltaTime);

        ImGui::Begin("Settings");
        ImGui::DragFloat2("Gravity", glm::value_ptr(mGravitationalAcceleration), 10.0f, -1'000.0f, 1'000.0f);
        ImGui::End();

        ImGui::Begin("Statistics");
        ImGui::Text("Frame Rate       %3.f Hz", 1.0f / deltaTime.asSeconds());
        ImGui::Text("Tick Rate        %3.f Hz", 1.0f / fixedDeltaTime.asSeconds());
        ImGui::Text("Fixed Delta Time %3d ms", fixedDeltaTime.asMilliseconds());
        ImGui::Text("Delta Time       %3d ms", deltaTime.asMilliseconds());
        ImGui::End();

        mWindow.clear(sf::Color::Black);

        for (const Object &object: mObjects) {
            sf::CircleShape circle2;
            circle2.setRadius(object.radius);
            circle2.setOrigin(object.radius, object.radius);
            circle2.setPosition(object.position.x, object.position.y);
            circle2.setFillColor(object.color);
            if (mIsObjectSelected && &object == &mObjects[mSelectedObjectIndex]) {
                circle2.setFillColor(sf::Color::White);
            } else {
                circle2.setFillColor(object.color);
            }
            mWindow.draw(circle2);
        }

        ImGui::SFML::Render(mWindow);

        mWindow.display();
    }

    ImGui::SFML::Shutdown();
}

void Application::ProcessEvents() {
    sf::Event event{};
    while (mWindow.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(mWindow, event);
        switch (event.type) {
            case sf::Event::Closed:
                HandleEventClosed(event);
                break;

            case sf::Event::Resized:
                HandleEventResized(event);
                break;

            case sf::Event::MouseWheelScrolled:
                HandleEventMouseWheelScrolled(event);
                break;

            case sf::Event::MouseButtonPressed:
                HandleEventMouseButtonPressed(event);
                break;

            case sf::Event::MouseButtonReleased:
                HandleEventMouseButtonReleased(event);
                break;

            default:
                break;
        }
    }
}

void Application::HandleEventClosed(const sf::Event &) {
    mWindow.close();
}

void Application::HandleEventResized(const sf::Event &event) {
    mView.setSize(static_cast<float>(event.size.width), static_cast<float>(event.size.height));
    mWindow.setView(mView);
}

void Application::HandleEventMouseWheelScrolled(const sf::Event &event) {
    if (event.mouseWheelScroll.delta > 0.0f) {
        mView.zoom(1.0f / 1.05f);
    }
    if (event.mouseWheelScroll.delta < 0.0f) {
        mView.zoom(1.05f);
    }
    mWindow.setView(mView);
}

void Application::HandleEventMouseButtonPressed(const sf::Event &event) {
    if (event.mouseButton.button == sf::Mouse::Left) {
        for (size_t objectIndex = 0; objectIndex < mObjects.size(); ++objectIndex) {
            const Object &object = mObjects[objectIndex];
            const sf::Vector2i pixelPosition = sf::Mouse::getPosition(mWindow);
            const sf::Vector2f worldPosition = mWindow.mapPixelToCoords(pixelPosition);
            const float distance = glm::length(glm::vec2(worldPosition.x - object.position.x, worldPosition.y - object.position.y));
            if (distance <= object.radius) {
                mSelectedObjectIndex = objectIndex;
                mIsObjectSelected = true;
                break;
            }
        }
    }
}

void Application::HandleEventMouseButtonReleased(const sf::Event &event) {
    if (event.mouseButton.button == sf::Mouse::Left) {
        mIsObjectSelected = false;
    }
}

void Application::FixedUpdate(const sf::Time &fixedDeltaTime) {
    const float dt = fixedDeltaTime.asSeconds();
    for (size_t i = 0; i < mObjects.size(); ++i) {
        Object &object1 = mObjects[i];
        for (size_t j = i + 1; j < mObjects.size(); ++j) {
            Object &object2 = mObjects[j];
            const glm::vec2 dPosition = object1.position - object2.position;
            if (const float dRadius = object1.radius + object2.radius; glm::length(dPosition) < dRadius) {
                const float delta = 0.5f * (glm::length(dPosition) - dRadius);
                const float massRatio1 = object1.radius / (object1.radius + object2.radius);
                const float massRatio2 = object2.radius / (object1.radius + object2.radius);
                object1.position -= glm::normalize(dPosition) * (massRatio2 * delta);
                object2.position += glm::normalize(dPosition) * (massRatio1 * delta);
            }
        }
    }
    for (Object &object: mObjects) {
        object.acceleration += mGravitationalAcceleration;
        const glm::vec2 dPosition = constraintPosition - object.position;
        if (const float dRadius = constraintRadius - object.radius; glm::length(dPosition) > dRadius) {
            object.position = constraintPosition - dRadius * glm::normalize(dPosition);
        }
        const glm::vec2 displacement = object.position - object.position_last;
        object.position_last = object.position;
        object.position = object.position + displacement + object.acceleration * dt * dt;
        object.acceleration = glm::vec2(0.0f, 0.0f);
    }
}