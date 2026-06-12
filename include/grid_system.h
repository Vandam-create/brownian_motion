#pragma once
#include <vector>
#include <cstddef>
#include <cmath>
#include "Vec2D.h"

class GridSystem {
private:
    double cell_size;
    int width_cells;
    int height_cells;

    // Сетка: двумерный массив ячеек, каждая ячейка хранит список индексов частиц
    std::vector<std::vector<std::vector<size_t>>> cells;

public:
    GridSystem() = default;
    
    // Инициализация размеров сетки на основе размеров бокса
    void init(double box_width, double box_height, double max_particle_diameter) {
        cell_size = max_particle_diameter;
        width_cells = std::max(1, static_cast<int>(std::ceil(box_width / cell_size)));
        height_cells = std::max(1, static_cast<int>(std::ceil(box_height / cell_size)));
        
        // Резервируем память под двумерную сетку
        cells.assign(width_cells, std::vector<std::vector<size_t>>(height_cells));
    }

    // Полная очистка всех ячеек перед каждым кадром
    void clear() {
        for (int x = 0; x < width_cells; ++x) {
            for (int y = 0; y < height_cells; ++y) {
                cells[x][y].clear();
            }
        }
    }

    // Добавление индекса частицы в соответствующую ячейку по её координатам
    void addParticle(size_t index, const Vec2d& pos) {
        int cx = static_cast<int>(pos.x / cell_size);
        int cy = static_cast<int>(pos.y / cell_size);

        // Защита от вылета частиц за пределы сетки (на всякий случай)
        if (cx < 0) cx = 0;
        if (cx >= width_cells) cx = width_cells - 1;
        if (cy < 0) cy = 0;
        if (cy >= height_cells) cy = height_cells - 1;

        cells[cx][cy].push_back(index);
    }

    // Геттеры для обхода сетки в физическом движке
    int getWidthCells() const { return width_cells; }
    int getHeightCells() const { return height_cells; }
    const std::vector<size_t>& getCellContent(int x, int y) const { return cells[x][y]; }
};
