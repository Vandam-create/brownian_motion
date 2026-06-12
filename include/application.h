#pragma once
#include <SFML/Graphics.hpp>
#include "physics_engine.h"
#include "unit_converter.h"

class Application {
private:
    sf::RenderWindow window;
    PhysicsEngine engine;
    UnitConverter converter;
    
    void spawnGas(int count, double temperature_si, double particle_mass_si, double particle_radius_si);

    //Массив вершин для отрисовки траектории броуновского тела
    sf::VertexArray brownian_track; 

    void processEvents();
    void update(float dt);
    void render();
    
public:
    Application();
    void run();
};
