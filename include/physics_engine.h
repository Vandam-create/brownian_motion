#ifndef PHYSICSENGINE
#define PHYSICSENGINE

#include <iostream>
#include <vector>
#include "Vec2D.h"
#include <memory>
#include "particle.h"
#include <algorithm>
#include "config.h"
#include "grid_system.h"

class PhysicsEngine {
private:
    std::vector<Particle> particles;
    GridSystem grid;

    Vec2d gravity{0.0, 0.0};
    double time_step = 0.016;
    double current_time = 0.0;
    int solver_iterations = 10;
    double damping = 0;
    
    Config::SimulationMode mode = Config::SimulationMode::BROWNIAN_MOTION;

    double box_width = 1600;
    double box_height = 800;
    

    void solveGridCollisions();
    void resolveParticleCollision(size_t idx1, size_t idx2);
    void solveWallCollisions();
public:
    PhysicsEngine() = default;
    PhysicsEngine(const PhysicsEngine &eng) = default;

    PhysicsEngine(const Vec2d& grav, double dt, int iter, double damp = 0)
        : gravity(grav), time_step(dt), solver_iterations(iter), damping(damp) {}
    
    //запрещаем копирование
    PhysicsEngine& operator=(const PhysicsEngine&) = delete;
    
    //разрешаем перемещение
    PhysicsEngine(PhysicsEngine&&) = default;
    PhysicsEngine& operator=(PhysicsEngine&&) = default;
    
    //создание частицы
    void initGridSystem(double max_diameter) {
        grid.init(box_width, box_height, max_diameter);
    }

    size_t createParticle(const Vec2d& position, double mass = 1.0, double radius = 5.0, Vec2d velosity = {0, 0}, bool fixed = false) {
        particles.emplace_back(position, mass, velosity, radius, fixed); // Скорость — 3, Радиус — 4
        return particles.size() - 1;
    }

    
    //удаление частицы
    void removeParticle(size_t idx);
        
    //основной метод симуляции "PBD" (вельвет говно)
    void step();
    
    //геттеры
    size_t getParticleCount() const { return particles.size(); }
    Particle& getParticle(size_t idx) { return particles[idx]; }
    const Particle& getParticle(size_t idx) const { return particles[idx]; }
    double getTime() const { return current_time; }
    double getTimeStep() const { return time_step; }
    
    //сеттеры
    void setBoxDimensions(double w, double h) { box_width = w; box_height = h; }
    void setGravity(const Vec2d& grav) { gravity = grav; }
    void setTimeStep(double dt) { if (dt > 0.0) time_step = dt; }
    void setSolverIterations(int iter) { if (iter > 0) solver_iterations = iter; }
    void setDamping(double damp) { damping = std::max(0.0, damp); }
    void setParticle(std::vector<Particle> setter) { particles = setter; }
    void reset_time(){current_time = 0;}


    void setMode(Config::SimulationMode new_mode) { mode = new_mode; }
    Config::SimulationMode getMode() const { return mode; }
    //очистка
    void clear() {
        particles.clear();
        current_time = 0.0;
    }

    //применение силы
    void applyForceToParticle(size_t idx, const Vec2d& force) {
        if (idx < particles.size()) {
            particles[idx].applyForce(force, time_step);
        }
    }
    
    //применение импульса
    void applyImpulseToParticle(size_t idx, const Vec2d& impulse) {
        if (idx < particles.size()) {
            Particle& p = particles[idx];
            if (!p.fixed) {
                p.velocity += impulse * p.inv_mass;
            }
        }
    }
};

#endif