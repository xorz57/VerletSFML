#pragma once

#include <SFML/Graphics.hpp>

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

class Solver
{
public:
    Solver() = default;

    VerletObject &addObject(sf::Vector2f position, float radius)
    {
        return mVerletObjects.emplace_back(position, radius);
    }

    void update()
    {
        mTime += mFrameDt;
        const float step_dt = getStepDt();
        for (uint32_t i{mSubSteps}; i--;)
        {
            applyGravity();
            checkCollisions(step_dt);
            applyConstraint();
            updateObjects(step_dt);
        }
    }

    void setSimulationUpdateRate(uint32_t rate)
    {
        mFrameDt = 1.0f / static_cast<float>(rate);
    }

    void setConstraint(sf::Vector2f position, float radius)
    {
        mConstraintCenter = position;
        mConstraintRadius = radius;
    }

    void setSubStepsCount(uint32_t sub_steps) { mSubSteps = sub_steps; }

    void setObjectVelocity(VerletObject &object, sf::Vector2f v) const
    {
        object.setVelocity(v, getStepDt());
    }

    [[nodiscard]] const std::vector<VerletObject> &getObjects() const
    {
        return mVerletObjects;
    }

    [[nodiscard]] sf::Vector3f getConstraint() const
    {
        return {mConstraintCenter.x, mConstraintCenter.y, mConstraintRadius};
    }

    [[nodiscard]] uint64_t getObjectsCount() const { return mVerletObjects.size(); }

    [[nodiscard]] float getTime() const { return mTime; }

    [[nodiscard]] float getStepDt() const
    {
        return mFrameDt / static_cast<float>(mSubSteps);
    }

private:
    void applyGravity()
    {
        for (auto &verletObject : mVerletObjects)
        {
            verletObject.acceleration += mGravitationalAcceleration;
        }
    }

    void checkCollisions(float)
    {
        const float response_coef = 0.75f;
        const uint64_t objects_count = mVerletObjects.size();
        // Iterate on all objects
        for (uint64_t i{0}; i < objects_count; ++i)
        {
            VerletObject &object_1 = mVerletObjects[i];
            // Iterate on object involved in new collision pairs
            for (uint64_t k{i + 1}; k < objects_count; ++k)
            {
                VerletObject &object_2 = mVerletObjects[k];
                const sf::Vector2f v = object_1.position - object_2.position;
                const float dist2 = v.x * v.x + v.y * v.y;
                const float min_dist = object_1.radius + object_2.radius;
                // Check overlapping
                if (dist2 < min_dist * min_dist)
                {
                    const float dist = sqrt(dist2);
                    const sf::Vector2f n = v / dist;
                    const float mass_ratio_1 =
                        object_1.radius / (object_1.radius + object_2.radius);
                    const float mass_ratio_2 =
                        object_2.radius / (object_1.radius + object_2.radius);
                    const float delta = 0.5f * response_coef * (dist - min_dist);
                    // Update positions
                    object_1.position -= n * (mass_ratio_2 * delta);
                    object_2.position += n * (mass_ratio_1 * delta);
                }
            }
        }
    }

    void applyConstraint()
    {
        for (auto &verletObject : mVerletObjects)
        {
            const sf::Vector2f v = mConstraintCenter - verletObject.position;
            const float dist = sqrt(v.x * v.x + v.y * v.y);
            if (dist > (mConstraintRadius - verletObject.radius))
            {
                const sf::Vector2f n = v / dist;
                verletObject.position =
                    mConstraintCenter - n * (mConstraintRadius - verletObject.radius);
            }
        }
    }

    void updateObjects(float dt)
    {
        for (auto &verletObject : mVerletObjects)
        {
            verletObject.update(dt);
        }
    }

    uint32_t mSubSteps = 1;
    sf::Vector2f mGravitationalAcceleration{0.0f, 1000.0f};
    sf::Vector2f mConstraintCenter{0.0f, 0.0f};
    float mConstraintRadius = 100.0f;
    std::vector<VerletObject> mVerletObjects;
    float mTime = 0.0f;
    float mFrameDt = 0.0f;
};
