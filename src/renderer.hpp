#pragma once

#include "solver.hpp"

class Renderer
{
public:
    explicit Renderer(sf::RenderTarget &renderTarget) : mRenderTarget{renderTarget} {}

    void render(const Solver &solver) const
    {
        // Render constraint
        const sf::Vector3f constraint = solver.getConstraint();
        sf::CircleShape constraint_background{constraint.z};
        constraint_background.setOrigin(constraint.z, constraint.z);
        constraint_background.setFillColor(sf::Color::Black);
        constraint_background.setPosition(constraint.x, constraint.y);
        constraint_background.setPointCount(128);
        mRenderTarget.draw(constraint_background);

        // Render objects
        sf::CircleShape circle{1.0f};
        circle.setPointCount(32);
        circle.setOrigin(1.0f, 1.0f);
        const auto &objects = solver.getObjects();
        for (const auto &obj : objects)
        {
            circle.setPosition(obj.position);
            circle.setScale(obj.radius, obj.radius);
            circle.setFillColor(obj.color);
            mRenderTarget.draw(circle);
        }
    }

private:
    sf::RenderTarget &mRenderTarget;
};
