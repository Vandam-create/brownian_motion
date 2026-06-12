#pragma once

namespace Config {
    // --- НАСТРОЙКИ ОКНА И ЭКРАНА ---
    inline constexpr int SCREEN_WIDTH = 1600;
    inline constexpr int SCREEN_HEIGHT = 800;
    inline constexpr double PIXEL_SCALE = 2.0; // Пикселей на единицу движка

    // --- НАСТРОЙКИ ФИЗИЧЕСКОГО БОКСА ---
    inline constexpr double BOX_WIDTH_ENGINE = 800.0;
    inline constexpr double BOX_HEIGHT_ENGINE = 400.0;
    inline constexpr double TIME_STEP = 0.016;
    inline constexpr double GRID_CELL_SIZE = 25.0;

    // --- ФИЗИЧЕСКИЕ КОНСТАНТЫ СИ ---
    inline constexpr double K_BOLTZMANN = 1.38e-23;

    // --- ПАРАМЕТРЫ МЕЛКОГО ГАЗА (АРГОН) ---
    inline constexpr int GAS_PARTICLE_COUNT = 250;
    inline constexpr double GAS_MASS_SI = 6.6e-26;
    inline constexpr double GAS_RADIUS_SI = 3.0e-10; // Честный физический радиус
    inline constexpr double GAS_TEMPERATURE_SI = 350.0;

    // --- ПАРАМЕТРЫ ТЯЖЕЛОГО БРОУНОВСКОГО ТЕЛА ---
    // Масса тяжелее газа в 1200 раз
    inline constexpr double BROWN_MASS_SI = GAS_MASS_SI * 1200.0; 
    inline constexpr double BROWN_RADIUS_SI = 1.2e-8; // 12 нанометров
}
