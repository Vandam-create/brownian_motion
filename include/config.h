#pragma once
#include <cmath>

namespace Config {
    // --- ПЕРЕЧИСЛЕНИЕ РЕЖИМОВ СИМУЛЯЦИИ ---
    enum class SimulationMode {
        BROWNIAN_MOTION,  // Режим 1: Броуновское движение
        EFFUSION          // Режим 2: Эффузия газа
    };

    // Текущий режим по умолчанию при старте приложения
    inline SimulationMode CURRENT_MODE = SimulationMode::BROWNIAN_MOTION;

    // --- НАСТРОЙКИ ОКНА И ЭКРАНА ---
    inline constexpr int SCREEN_WIDTH = 1600;
    inline constexpr int SCREEN_HEIGHT = 800;
    inline constexpr double PIXEL_SCALE = 2.0; 

    // --- НАСТРОЙКИ ФИЗИЧЕСКОГО БОКСА ---
    inline constexpr double BOX_WIDTH_ENGINE = 800.0;
    inline constexpr double BOX_HEIGHT_ENGINE = 400.0;
    inline constexpr double TIME_STEP = 0.016;
    inline constexpr double GRID_CELL_SIZE = 25.0;

    // --- НАСТРОЙКИ ПЕРЕГОРОДКИ ДЛЯ ЭФФУЗИИ (В ед. движка) ---
    inline constexpr double WALL_X = BOX_WIDTH_ENGINE / 2.0; // Перегородка ровно по центру (X = 400)
    inline constexpr double HOLE_SIZE = 40.0;               // Размер дырки (40 единиц)
    inline constexpr double HOLE_TOP = (BOX_HEIGHT_ENGINE - HOLE_SIZE) / 2.0;    // Y = 180
    inline constexpr double HOLE_BOTTOM = (BOX_HEIGHT_ENGINE + HOLE_SIZE) / 2.0; // Y = 220

    // --- ФИЗИЧЕСКИЕ КОНСТАНТЫ СИ ---
    inline constexpr double K_BOLTZMANN = 1.38e-23;

    // =========================================================================
    // БЛОК 1: НАСТРОЙКИ ДЛЯ БРОУНОВСКОГО ДВИЖЕНИЯ (ДИФФУЗИЯ)
    // =========================================================================
    // Параметры подобраны так, чтобы графики идеально ложились на теорию
    inline constexpr int BR_GAS_COUNT = 850;          
    inline constexpr double BR_GAS_MASS_SI = 6.6e-26;
    inline constexpr double BR_GAS_RADIUS_SI = 4.0e-10; 
    inline constexpr double BR_GAS_TEMP_SI = 450.0;    

    // Параметры тяжелого Броуновского тела (из твоего исходного конфига)
    inline constexpr double BROWN_MASS_SI = BR_GAS_MASS_SI * 1200.0; 
    inline constexpr double BROWN_RADIUS_SI = 1.2e-8; 

        // =========================================================================
    // БЛОК 2: НАСТРОЙКИ ДЛЯ ЭФФУЗИИ ГАЗА
    // =========================================================================
    inline constexpr int EFF_GAS_COUNT = 250;
    inline constexpr double EFF_GAS_MASS_SI = 6.6e-26;
    inline constexpr double EFF_GAS_RADIUS_SI = 3.0e-10; 
    inline constexpr double EFF_GAS_TEMP_SI = 350.0;


    // =========================================================================
    // РАСЧЕТ ТЕОРЕТИЧЕСКОГО КОЭФФИЦИЕНТА ДИФФУЗИИ D (ДЛЯ БЛОКА БРОУНОВСКОГО ДВИЖЕНИЯ)
    // =========================================================================
    inline const double GAS_CONCENTRATION_ENGINE = BR_GAS_COUNT / (BOX_WIDTH_ENGINE * BOX_HEIGHT_ENGINE);
    inline const double GAS_V_MEAN_ENGINE = std::sqrt(2.0 * K_BOLTZMANN * BR_GAS_TEMP_SI / BR_GAS_MASS_SI);
    inline const double COLLISION_CROSS_SECTION = 2.0 * (BROWN_RADIUS_SI + BR_GAS_RADIUS_SI) * 5e8; 
    inline const double COLLISION_FREQUENCY = GAS_CONCENTRATION_ENGINE * COLLISION_CROSS_SECTION * GAS_V_MEAN_ENGINE;
    inline const double DIFFUSION_D_ENGINE = (GAS_V_MEAN_ENGINE / COLLISION_FREQUENCY) / 2.0;
}
