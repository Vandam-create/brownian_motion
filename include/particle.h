#pragma once
#include "Vec2D.h"

struct Particle{
    Vec2d position;
    Vec2d predicted_position;
    Vec2d velocity;
    double inv_mass;
    double radius;
    bool fixed;

    void setMass(double mass) {
        if (mass <= 0.0) {
            inv_mass = 0.0;
        } else {
            inv_mass = 1.0 / mass;
        }
        if (fixed) {
            inv_mass = 0.0;
        }
    }

    Particle(const Vec2d pos = {0, 0}, double mass = 1.0, Vec2d vel = {0 , 0}, double radius = 5.0, bool is_fixed = false)
        : position(pos), predicted_position(pos), velocity(vel), radius(radius), fixed(is_fixed) {
        setMass(mass);
    }
    
    void set_velocity(Vec2d vel){velocity = vel;}

    void applyForce(const Vec2d& force, double dt) {
        if (!fixed) {
            velocity += force * inv_mass * dt;
        }
    }
};