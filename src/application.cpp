#include "application.h"
#include "config.h"
#include <random>
#include <cmath>

Application::Application() {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    window.create(sf::VideoMode(screen_width, screen_height), "PBD Gas & Brownian Motion Verification", sf::Style::Default, settings);
    window.setFramerateLimit(60);

    converter.setScreenScale(Config::PIXEL_SCALE);
    engine.setBoxDimensions(box_width_engine, box_height_engine);
    engine.setTimeStep(Config::TIME_STEP);
    engine.initGridSystem(Config::GRID_CELL_SIZE);

    resetSimulation();
}

void Application::resetSimulation() {
    engine.clear();
    experimental_r.clear();
    theoretical_r.clear();
    brownian_track.clear();
    
    //сброс данных эффузии
    effusion_N_left.clear();
    total_effusion_energy = 0.0;
    effusion_exit_count = 0;

    engine.reset_time();
    engine.setMode(Config::CURRENT_MODE);

    initial_brownian_pos = Vec2d(box_width_engine / 2.0, box_height_engine / 2.0);

    if (Config::CURRENT_MODE == Config::SimulationMode::BROWNIAN_MOTION) {
        //режим Броуновского движения
        engine.createParticle(initial_brownian_pos, converter.massSIToEngine(Config::BROWN_MASS_SI), Config::BROWN_RADIUS_SI * 5e8, Vec2d(0, 0), false);
        
        spawnGas(Config::BR_GAS_COUNT, Config::BR_GAS_TEMP_SI, Config::BR_GAS_MASS_SI, Config::BR_GAS_RADIUS_SI);
    } 
    else if (Config::CURRENT_MODE == Config::SimulationMode::EFFUSION) {
        //режим Эффузии

        spawnGas(Config::EFF_GAS_COUNT, Config::EFF_GAS_TEMP_SI, Config::EFF_GAS_MASS_SI, Config::EFF_GAS_RADIUS_SI);
    }

    

}

void Application::spawnGas(int count, double temperature_si, double particle_mass_si, double particle_radius_si) {
    std::random_device rd;
    std::mt19937 gen(rd());

    double m_engine = converter.massSIToEngine(particle_mass_si);
    double r_engine = converter.lengthSIToEngine(particle_radius_si);

    double v_std_si = std::sqrt((Config::K_BOLTZMANN * temperature_si) / particle_mass_si);
    double v_std_engine = converter.velSIToEngine(v_std_si);

    std::normal_distribution<double> rand_velocity(0.0, v_std_engine);
    
    double max_spawn_x = box_width_engine - r_engine;
    if (Config::CURRENT_MODE == Config::SimulationMode::EFFUSION) {
        max_spawn_x = Config::WALL_X - r_engine - 5.0; 
    }

    std::uniform_real_distribution<double> rand_x(r_engine, max_spawn_x);
    std::uniform_real_distribution<double> rand_y(r_engine, box_height_engine - r_engine);

    for (int i = 0; i < count; ++i) {
        Vec2d pos(rand_x(gen), rand_y(gen));
        
        if (Config::CURRENT_MODE == Config::SimulationMode::BROWNIAN_MOTION) {
            double dx = pos.x - initial_brownian_pos.x;
            double dy = pos.y - initial_brownian_pos.y;
            while ((dx * dx + dy * dy) < 400.0) { 
                pos = Vec2d(rand_x(gen), rand_y(gen));
                dx = pos.x - initial_brownian_pos.x;
                dy = pos.y - initial_brownian_pos.y;
            }
        }
        
        Vec2d vel(rand_velocity(gen), rand_velocity(gen));
        engine.createParticle(pos, m_engine, r_engine, vel, false);
    }
}

void Application::run() {
    double track_timer = 0.0;
    const double track_interval = 0.1;

    brownian_track.setPrimitiveType(sf::LineStrip);

    while (window.isOpen()) {
        processEvents();
        engine.step();

        //СБОР АНАЛИТИКИ БРОУНОВСКОГО ДВИЖЕНИЯ
        if (Config::CURRENT_MODE == Config::SimulationMode::BROWNIAN_MOTION && engine.getParticleCount() > 0) {
            Vec2d current_pos = engine.getParticle(0).position;
            double t = engine.getTime();

            double dx = current_pos.x - initial_brownian_pos.x;
            double dy = current_pos.y - initial_brownian_pos.y;
            experimental_r.push_back(std::sqrt(dx * dx + dy * dy));
            theoretical_r.push_back(std::sqrt(4 * Config::DIFFUSION_D_ENGINE * t));

            track_timer += engine.getTimeStep();
            if (track_timer >= track_interval) {
                track_timer = 0.0;
                Vec2d pix_pos = converter.coordsToPixels(current_pos);
                brownian_track.append(sf::Vertex(sf::Vector2f(static_cast<float>(pix_pos.x), static_cast<float>(pix_pos.y)), sf::Color(255, 215, 0, 150)));
            }
        }
        //СБОР АНАЛИТИКИ ЭФФУЗИИ ГАЗА С ИСЧЕЗНОВЕНИЕМ ЧАСТИЦ
        else if (Config::CURRENT_MODE == Config::SimulationMode::EFFUSION) {
            int left_count = 0;

            //идем по массиву частиц С КОНЦА (обратный цикл), чтобы при удалении частиц индексы не ломались(по иному будет сложно)
            size_t p_count = engine.getParticleCount();
            for (size_t i = p_count; i > 0; --i) {
                size_t idx = i - 1;
                const auto& p = engine.getParticle(idx);

                //если частица всё еще в левой половине — просто считаем её
                if (p.position.x < Config::WALL_X) {
                    left_count++;
                } 
                //ЕСЛИ ЧАСТИЦА ПЕРЕСЕКЛА ЛИНЮ ПЕРЕГОРОДКИ (ВЫЛЕТЕЛА!)
                else {
                    // 1. Считаем её кинетическую энергию в момент вылета
                    double v_sq = p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y;
                    double mass = (p.inv_mass > 0.0) ? (1.0 / p.inv_mass) : 1.0;
                    double e_k = 0.5 * mass * v_sq;


                    // Нам нужно посчитать среднюю энергию газа внутри ИМЕННО В ЭТОТ КАДР, чтобы нормировать вылет
                    double current_internal_energy_sum = 0.0;
                    int current_left_count = 0;
                    for (size_t j = 0; j < p_count; ++j) {
                        const auto& internal_p = engine.getParticle(j);
                        if (internal_p.position.x < Config::WALL_X) {
                            double i_v_sq = internal_p.velocity.x * internal_p.velocity.x + internal_p.velocity.y * internal_p.velocity.y;
                            double i_mass = (internal_p.inv_mass > 0.0) ? (1.0 / internal_p.inv_mass) : 1.0;
                            current_internal_energy_sum += 0.5 * i_mass * i_v_sq;
                            current_left_count++;
                        }
                    }
                    double current_avg_internal = (current_left_count > 0) ? (current_internal_energy_sum / current_left_count) : 1.0;

                    // Накапливаем не абсолютную энергию, а нормированное отношение прямо в момент вылета!
                    total_effusion_energy += (e_k / current_avg_internal); 
                    effusion_exit_count++;        

                    //мгновенно удаляем частицу
                    engine.removeParticle(idx);
                }

            }
            
            //записываем текущее количество оставшихся частиц для зеленого графика
            effusion_N_left.push_back(static_cast<double>(left_count));
        }

        render();
    }
}



void Application::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
            
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Space) {
                resetSimulation();
            }
            if (event.key.code == sf::Keyboard::Tab) {
                is_menu_open = !is_menu_open;
            }
            if (event.key.code == sf::Keyboard::M) {
                if (Config::CURRENT_MODE == Config::SimulationMode::BROWNIAN_MOTION) {
                    Config::CURRENT_MODE = Config::SimulationMode::EFFUSION;
                } else {
                    Config::CURRENT_MODE = Config::SimulationMode::BROWNIAN_MOTION;
                }
                resetSimulation(); 
            }
        }
    }
}

void Application::render() {
    window.clear(sf::Color(10, 10, 18));

    if (Config::CURRENT_MODE == Config::SimulationMode::BROWNIAN_MOTION && brownian_track.getVertexCount() > 0) {
        window.draw(brownian_track);
    }

    if (Config::CURRENT_MODE == Config::SimulationMode::EFFUSION) {
        sf::RectangleShape wall_shape;
        wall_shape.setFillColor(sf::Color(150, 150, 150, 200)); 
        wall_shape.setSize(sf::Vector2f(4.0f, static_cast<float>(Config::HOLE_TOP * Config::PIXEL_SCALE)));
        
        wall_shape.setPosition(static_cast<float>(Config::WALL_X * Config::PIXEL_SCALE) - 2.0f, 0.0f);
        window.draw(wall_shape);

        float bottom_wall_y = static_cast<float>(Config::HOLE_BOTTOM * Config::PIXEL_SCALE);
        wall_shape.setSize(sf::Vector2f(4.0f, static_cast<float>((Config::BOX_HEIGHT_ENGINE - Config::HOLE_BOTTOM) * Config::PIXEL_SCALE)));
        wall_shape.setPosition(static_cast<float>(Config::WALL_X * Config::PIXEL_SCALE) - 2.0f, bottom_wall_y);
        window.draw(wall_shape);
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

        if (Config::CURRENT_MODE == Config::SimulationMode::BROWNIAN_MOTION && i == 0) {
            shape.setFillColor(sf::Color(255, 120, 0)); 
        } else {
            shape.setFillColor(sf::Color(100, 180, 255, 220)); 
        }

        window.draw(shape);
    }

    if (is_menu_open) {
        renderMenu();
    }

    window.display();
}

void Application::renderMenu() {
    //создание расширенную полупрозрачную подложку меню (ширина 750 пикселей)
    sf::RectangleShape menu_box(sf::Vector2f(750.0f, 260.0f));
    menu_box.setPosition(30.0f, 30.0f);
    menu_box.setFillColor(sf::Color(25., 25.0f, 35.0f, 240)); // Глубокий темный цвет
    menu_box.setOutlineThickness(2.0f);
    menu_box.setOutlineColor(sf::Color::White);
    window.draw(menu_box);

    //загружаю шрифты
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")) {
        // Резервный путь, если первого шрифта вдруг нет
        font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    }

    //текст
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(16);
    text.setFillColor(sf::Color::White);

    //Координаты начала осей графика
    float ox = 70.0f;
    float oy = 240.0f;
    
    //Рисуем рамку осей координат графика
    sf::VertexArray axes(sf::Lines, 4);
    axes[0] = sf::Vertex(sf::Vector2f(ox, oy), sf::Color::White);       
    axes[1] = sf::Vertex(sf::Vector2f(ox + 350.0f, oy), sf::Color::White); // Ось X (350 пикс)
    axes[2] = sf::Vertex(sf::Vector2f(ox, oy), sf::Color::White);       
    axes[3] = sf::Vertex(sf::Vector2f(ox, oy - 180.0f), sf::Color::White); // Ось Y (180 пикс)
    window.draw(axes);

    //Подписи к осям графика
    text.setString("t");
    text.setPosition(ox + 360.0f, oy - 10.0f);
    window.draw(text);

    //ОТРИСОВКА ДАННЫХ ДЛЯ РЕЖИМА БРОУНОВСКОГО ДВИЖЕНИЯ
    if (Config::CURRENT_MODE == Config::SimulationMode::BROWNIAN_MOTION) {
        //заголовок ht;bvf
        text.setString("Mode: Brownian Motion");
        text.setPosition(460.0f, 50.0f);
        text.setFillColor(sf::Color(255, 165, 0)); // Оранжевый
        window.draw(text);
        text.setFillColor(sf::Color::White);

        if (experimental_r.empty()) return;

        size_t points_count = experimental_r.size();
        double max_val = 1.0; 
        for (size_t i = 0; i < points_count; ++i) {
            if (experimental_r[i] > max_val) max_val = experimental_r[i];
            if (theoretical_r[i] > max_val)  max_val = theoretical_r[i];
        }

        float scale_x = 330.0f / static_cast<float>(points_count);
        float scale_y = 160.0f / static_cast<float>(max_val);

        sf::VertexArray exp_line(sf::LineStrip, points_count);
        sf::VertexArray theo_line(sf::LineStrip, points_count);

        for (size_t i = 0; i < points_count; ++i) {
            float px = ox + static_cast<float>(i) * scale_x;
            float py_exp = oy - static_cast<float>(experimental_r[i]) * scale_y;
            exp_line[i] = sf::Vertex(sf::Vector2f(px, py_exp), sf::Color::Yellow);

            float py_theo = oy - static_cast<float>(theoretical_r[i]) * scale_y;
            theo_line[i] = sf::Vertex(sf::Vector2f(px, py_theo), sf::Color(255, 50, 50));
        }
        window.draw(exp_line);
        window.draw(theo_line);

        //текстовая аналитика справа от графика
        text.setString("Graph Legend:");
        text.setPosition(460.0f, 90.0f);
        window.draw(text);

        text.setString("- Yellow: Experiment r(t)");
        text.setPosition(460.0f, 120.0f);
        text.setFillColor(sf::Color::Yellow);
        window.draw(text);

        text.setString("- Red: Einstein Theory");
        text.setPosition(460.0f, 140.0f);
        text.setFillColor(sf::Color(255, 50, 50));
        window.draw(text);
        text.setFillColor(sf::Color::White);

        text.setString("Diffusion Coeff D: " + std::to_string(Config::DIFFUSION_D_ENGINE).substr(0, 5));
        text.setPosition(460.0f, 180.0f);
        window.draw(text);
        
        text.setString("Press 'M' to switch mode");
        text.setPosition(460.0f, 210.0f);
        text.setFillColor(sf::Color(150, 150, 150));
        window.draw(text);
    }   
    else if (Config::CURRENT_MODE == Config::SimulationMode::EFFUSION) {
        text.setString("N_left");
        text.setPosition(40.0f, 40.0f);
        window.draw(text);

        if (effusion_N_left.empty()) return;

        size_t points_count = effusion_N_left.size();
        float scale_x = 330.0f / static_cast<float>(points_count);
        float scale_y = 160.0f / static_cast<float>(Config::EFF_GAS_COUNT);

        sf::VertexArray n_left_line(sf::LineStrip, points_count);
        for (size_t i = 0; i < points_count; ++i) {
            float px = ox + static_cast<float>(i) * scale_x;
            float py_n = oy - static_cast<float>(effusion_N_left[i]) * scale_y;
            n_left_line[i] = sf::Vertex(sf::Vector2f(px, py_n), sf::Color::Green);
        }
        window.draw(n_left_line);

        //выводим честный счетчик безвозвратно улетевших молекул
        text.setString("Particles Leaked: " + std::to_string(effusion_exit_count));
        text.setPosition(460.0f, 90.0f);
        window.draw(text);

        //считаем среднюю энергию частиц, оставшихся внутри левого бокса прямо сейчас
        double current_left_energy_sum = 0.0;
        int left_count = 0;
        size_t p_count = engine.getParticleCount();
        for (size_t i = 0; i < p_count; ++i) {
            const auto& p = engine.getParticle(i);
            double v_sq = p.velocity.x * p.velocity.x + p.velocity.y * p.velocity.y;
            double mass = (p.inv_mass > 0.0) ? (1.0 / p.inv_mass) : 1.0;
            current_left_energy_sum += 0.5 * mass * v_sq;
            left_count++;
        }
        double avg_internal_energy = (left_count > 0) ? (current_left_energy_sum / left_count) : 1.0;

        double live_ratio = 0.0;
        if (effusion_exit_count > 0) {
            // Так как мы в каждом кадре копили (E_выл / E_внутри), среднее арифметическое и есть наш коэффициент!
            live_ratio = total_effusion_energy / effusion_exit_count;
        }


        //Форматирование double строки
        std::string ratio_str = std::to_string(live_ratio);
        std::replace(ratio_str.begin(), ratio_str.end(), ',', '.'); 
        if (ratio_str.length() > 4) ratio_str = ratio_str.substr(0, 4);

        text.setString("E_out / E_inside ratio: " + ratio_str);
        text.setPosition(460.0f, 120.0f);
        text.setFillColor(sf::Color::Yellow);
        window.draw(text);
        text.setFillColor(sf::Color::White);

        text.setString("Theoretical Value: 1.50");
        text.setPosition(460.0f, 140.0f);
        text.setFillColor(sf::Color(150, 150, 150));
        window.draw(text);
        text.setFillColor(sf::Color::White);

        //Расчет погрешности эксперимента
        double error_percent = 100.0;
        if (live_ratio > 0.001) {
            error_percent = std::abs(live_ratio - 1.5) / 1.5 * 100.0;
        }
        std::string err_str = std::to_string(error_percent);
        std::replace(err_str.begin(), err_str.end(), ',', '.');
        if (err_str.length() > 4) err_str = err_str.substr(0, 4);
        
        text.setString("Current Error: " + err_str + " %");
        text.setPosition(460.0f, 170.0f);
        window.draw(text);

        text.setString("Press 'M' to switch mode");
        text.setPosition(460.0f, 210.0f);
        text.setFillColor(sf::Color(150, 150, 150));
        window.draw(text);
    }

}