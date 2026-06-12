#include "application.h"
#include "config.h" // ПОДКЛЮЧАЕМ НАШ КОНФИГ
#include <random>
#include <cmath>

Application::Application() {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    window.create(sf::VideoMode(Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT), "PBD Brownian Motion & Gas Simulation", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    // Применяем масштабы из конфига
    converter.setScreenScale(Config::PIXEL_SCALE);
    engine.setBoxDimensions(Config::BOX_WIDTH_ENGINE, Config::BOX_HEIGHT_ENGINE);
    engine.setTimeStep(Config::TIME_STEP);
    engine.initGridSystem(Config::GRID_CELL_SIZE);

    // 1. Спавним тяжелое Броуновское тело строго по центру бокса
    Vec2d center_engine(Config::BOX_WIDTH_ENGINE / 2.0, Config::BOX_HEIGHT_ENGINE / 2.0);
    
    // Броуновское тело всегда будет под индексом 0
    engine.createParticle(
        center_engine, 
        converter.massSIToEngine(Config::BROWN_MASS_SI), 
        Vec2d(0, 0), 
        converter.lengthSIToEngine(Config::BROWN_RADIUS_SI), 
        false
    );

    // 2. Спавним разреженный газ из конфига
    spawnGas(Config::GAS_PARTICLE_COUNT, Config::GAS_TEMPERATURE_SI, Config::GAS_MASS_SI, Config::GAS_RADIUS_SI);
    brownian_track.setPrimitiveType(sf::LineStrip);
}

void Application::spawnGas(int count, double temperature_si, double particle_mass_si, double particle_radius_si) {
    std::random_device rd;
    std::mt19937 gen(rd());

    double m_engine = converter.massSIToEngine(particle_mass_si);
    double r_engine = converter.lengthSIToEngine(particle_radius_si);

    // Расчет тепловой скорости Максвелла-Гаусса
    double v_std_si = std::sqrt((Config::K_BOLTZMANN * temperature_si) / particle_mass_si);
    double v_std_engine = converter.velSIToEngine(v_std_si);

    std::normal_distribution<double> rand_velocity(0.0, v_std_engine);
    std::uniform_real_distribution<double> rand_x(r_engine, Config::BOX_WIDTH_ENGINE - r_engine);
    std::uniform_real_distribution<double> rand_y(r_engine, Config::BOX_HEIGHT_ENGINE - r_engine);

    Vec2d center_engine(Config::BOX_WIDTH_ENGINE / 2.0, Config::BOX_HEIGHT_ENGINE / 2.0);

    for (int i = 0; i < count; ++i) {
        Vec2d pos(rand_x(gen), rand_y(gen));
        
        // Считаем квадрат расстояния вручную по Пифагору
        double dx = pos.x - center_engine.x;
        double dy = pos.y - center_engine.y;
        
        // Если молекула заспавнилась ближе чем на 20 единиц от центра тяжелого тела — перевыбираем позицию
        while ((dx * dx + dy * dy) < 400.0) { 
            pos = Vec2d(rand_x(gen), rand_y(gen));
            dx = pos.x - center_engine.x;
            dy = pos.y - center_engine.y;
        }
        
        Vec2d vel(rand_velocity(gen), rand_velocity(gen));
        engine.createParticle(pos, m_engine, vel, r_engine, false);
    }
}

void Application::run() {
    double track_timer = 0.0;
    const double track_interval = 0.1; // Фиксируем точку каждые 0.1 секунды времени движка

    while (window.isOpen()) {
        processEvents();
        
        engine.step();
        
        // Берем позицию Броуновского тела (индекс 0)
        Vec2d b_pos = engine.getParticle(0).position;

        // Таймер для записи точек траектории
        track_timer += engine.getTimeStep();
        if (track_timer >= track_interval) {
            track_timer = 0.0;
            
            // Переводим координаты в пиксели экрана
            Vec2d pix_pos = converter.coordsToPixels(b_pos);
            
            // Добавляем новую вершину ярко-желтого цвета
            brownian_track.append(sf::Vertex(sf::Vector2f(pix_pos.x, pix_pos.y), sf::Color(255, 215, 0, 180)));
        }
        
        render();
    }
}


void Application::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
            
        // Обработка кнопки пробела для возврата Броуновского тела в центр
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Space) {
                Vec2d center_engine(Config::BOX_WIDTH_ENGINE / 2.0, Config::BOX_HEIGHT_ENGINE / 2.0);
                auto& brownian = engine.getParticle(0);
                
                brownian.position = center_engine;
                brownian.predicted_position = center_engine;
                brownian.velocity = Vec2d(0, 0);

                // ДОБАВИТЬ: Очищаем старый след при перезапуске эксперимента
                brownian_track.clear(); 
            }
        
        }
    }
}

void Application::render() {
    window.clear(sf::Color(10, 10, 18));


    if (brownian_track.getVertexCount() > 0) {
        window.draw(brownian_track);
    }
    
    sf::CircleShape shape;
    size_t p_count = engine.getParticleCount();
    
    for (size_t i = 0; i < p_count; ++i) {
        const auto& particle = engine.getParticle(i);
        
        Vec2d pix_pos = converter.coordsToPixels(particle.position);
        double pix_radius = converter.toPixels(particle.radius);

        shape.setRadius(static_cast<float>(pix_radius));
        shape.setOrigin(static_cast<float>(pix_radius), static_cast<float>(pix_radius));
        shape.setPosition(static_cast<float>(pix_pos.x), static_cast<float>(pix_pos.y));

        // Выделяем Броуновское тело (индекс 0) оранжевым цветом, а газ — голубым
        if (i == 0) {
            shape.setFillColor(sf::Color(255, 120, 0)); // Оранжевый шар
        } else {
            shape.setFillColor(sf::Color(100, 180, 255, 220)); // Голубой газ
        }

        window.draw(shape);
    }

    window.display();
}
