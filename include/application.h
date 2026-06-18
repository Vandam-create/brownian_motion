#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "physics_engine.h"
#include "unit_converter.h"

class Application {
private:
    sf::RenderWindow window;
    PhysicsEngine engine;
    UnitConverter converter;

    //геометрия экрана
    const int screen_width = 1600;
    const int screen_height = 800;
    const double box_width_engine = 800.0;
    const double box_height_engine = 400.0;

    //логика всплывающего меню
    bool is_menu_open = true;

    //данные для графиков аналитики
    Vec2d initial_brownian_pos;
    std::vector<double> experimental_r; 
    std::vector<double> theoretical_r;    
    //ЭФФУЗИЯ

    std::vector<double> effusion_N_left; //История количества частиц в левой половине
    double total_effusion_energy = 0.0;  //Суммарная энергия всех вылетевших частиц
    int effusion_exit_count = 0;         //Количество вылетевших частиц

    //Массив вершин для отрисовки следа траектории
    sf::VertexArray brownian_track; 

    void processEvents();
    void update(float dt);
    void render();
    void renderMenu(); 
    
    void resetSimulation();
    void spawnGas(int count, double temperature_si, double particle_mass_si, double particle_radius_si);


public:
    Application();
    void run();
};
