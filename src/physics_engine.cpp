#include "physics_engine.h"
#include <cmath>
#include <algorithm>

// Главный такт симуляции
void PhysicsEngine::step() {
    //Внешние силы
    for (auto& particle : particles) {
        if (!particle.fixed) {
            particle.velocity += gravity * time_step;
            particle.velocity *= (1.0 - damping);
        }
    }
    
    //Прогноз позиций
    for (auto& particle : particles) {
        particle.predicted_position = particle.position + particle.velocity * time_step;
    }

    //перестроение сетки под новые прогнозируемые позиции кадра
    grid.clear();
    for (size_t i = 0; i < particles.size(); ++i) {
        grid.addParticle(i, particles[i].predicted_position);
    }

    //Итерационный солвер коллизий
    for (int iter = 0; iter < solver_iterations; ++iter) {
        solveGridCollisions();
        solveWallCollisions();
    }

    //финализация кадра
    for (auto& particle : particles) {
        if (!particle.fixed) {
            particle.position = particle.predicted_position;
        }
    }
    
    current_time += time_step;
}

void PhysicsEngine::solveGridCollisions() {
    int w_cells = grid.getWidthCells();
    int h_cells = grid.getHeightCells();

    const int dx[] = {1, 0, 1, -1};
    const int dy[] = {0, 1, 1,  1};

    for (int x = 0; x < w_cells; ++x) {
        for (int y = 0; y < h_cells; ++y) {
            const auto& cell = grid.getCellContent(x, y);
            if (cell.empty()) continue;

            for (size_t i = 0; i < cell.size(); ++i) {
                for (size_t j = i + 1; j < cell.size(); ++j) {
                    resolveParticleCollision(cell[i], cell[j]);
                }
            }

            for (size_t i = 0; i < cell.size(); ++i) {
                size_t idx1 = cell[i];
                
                for (int n = 0; n < 4; ++n) {
                    int nx = x + dx[n];
                    int ny = y + dy[n];

                    if (nx >= 0 && nx < w_cells && ny >= 0 && ny < h_cells) {
                        const auto& neighbor_cell = grid.getCellContent(nx, ny);
                        for (size_t idx2 : neighbor_cell) {
                            resolveParticleCollision(idx1, idx2);
                        }
                    }
                }
            }
        }
    }
}

//модифицированный PBD
void PhysicsEngine::resolveParticleCollision(size_t idx1, size_t idx2) {
    Particle& p1 = particles[idx1];
    Particle& p2 = particles[idx2];

    Vec2d delta = p2.predicted_position - p1.predicted_position;
    double dist_sq = delta.x * delta.x + delta.y * delta.y;
    double min_dist = p1.radius + p2.radius;

    if (dist_sq >= min_dist * min_dist || dist_sq < 1e-12) return;

    double dist = std::sqrt(dist_sq);
    Vec2d norm = delta / dist;

    double overlap = min_dist - dist;
    double total_weight = p1.inv_mass + p2.inv_mass;
    if (total_weight < 1e-12) return;

    Vec2d correction = norm * (overlap / total_weight);
    
    if (!p1.fixed) p1.predicted_position -= correction * p1.inv_mass;
    if (!p2.fixed) p2.predicted_position += correction * p2.inv_mass;

    double v1n = p1.velocity.x * norm.x + p1.velocity.y * norm.y;
    double v2n = p2.velocity.x * norm.x + p2.velocity.y * norm.y;
    double vRel = v1n - v2n;

    if (vRel > 0.0) {
        double impulse = (2.0 * vRel) / total_weight;
        p1.velocity -= norm * (impulse * p1.inv_mass);
        p2.velocity += norm * (impulse * p2.inv_mass);
    }
}

void PhysicsEngine::solveWallCollisions() {
    for (auto& p : particles) {
        if (p.fixed) continue;

        if (p.predicted_position.x - p.radius < 0.0) {
            p.predicted_position.x = p.radius;
            if (p.velocity.x < 0.0) p.velocity.x = -p.velocity.x;
        } else if (p.predicted_position.x + p.radius > box_width) {
            p.predicted_position.x = box_width - p.radius;
            if (p.velocity.x > 0.0) p.velocity.x = -p.velocity.x;
        }

        if (p.predicted_position.y - p.radius < 0.0) {
            p.predicted_position.y = p.radius;
            if (p.velocity.y < 0.0) p.velocity.y = -p.velocity.y;
        } else if (p.predicted_position.y + p.radius > box_height) {
            p.predicted_position.y = box_height - p.radius;
            if (p.velocity.y > 0.0) p.velocity.y = -p.velocity.y;
        }

        if (mode == Config::SimulationMode::EFFUSION) {
            double k = p.radius;
            
            if (std::abs(p.predicted_position.x - Config::WALL_X) < p.radius) {
                //ПРОВЕРЯЕМ: ПОПАЛИ ЛИ МЫ В ДЫРКУ?
                //tсли координата Y частицы лежит внутри дырки — она пролетает свободно
                if (p.predicted_position.y > Config::HOLE_TOP && p.predicted_position.y < Config::HOLE_BOTTOM) {
                    continue; 
                }
                
                if (p.position.x < Config::WALL_X) {
                    //летела слева -> возвращаем налево
                    p.predicted_position.x = Config::WALL_X - p.radius;
                    if (p.velocity.x > 0.0) p.velocity.x = -p.velocity.x;
                } else {
                    //летела справа -> возвращаем направо
                    p.predicted_position.x = Config::WALL_X + p.radius;
                    if (p.velocity.x < 0.0) p.velocity.x = -p.velocity.x;
                }
            }
        }
    }
}


void PhysicsEngine::removeParticle(size_t idx) {
    if (idx < particles.size()) {
        particles.erase(particles.begin() + idx);
    }
}
