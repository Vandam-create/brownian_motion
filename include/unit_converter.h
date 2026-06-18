#pragma once
#include "Vec2D.h"

class UnitConverter {
private:
    const double SI_to_Engine_Length = 5e8;     //1 метр СИ = 5*10^8 ед. движка
    const double SI_to_Engine_Mass   = 1e24;    //1 кг СИ = 10^24 ед. движка
    const double SI_to_Engine_Time   = 5e8;     //5e8 (чтобы скорость в движке была 1 к 1)

    //rоэффициент пересчета Движок -> Экран
    double engine_to_pixels = 4.0;

public:
    UnitConverter() = default;

    double lengthSIToEngine(double length_si) const { return length_si * SI_to_Engine_Length; }
    double massSIToEngine(double mass_si) const     { return mass_si * SI_to_Engine_Mass; }
    double timeSIToEngine(double time_si) const     { return time_si * SI_to_Engine_Time; }
    
    double velSIToEngine(double vel_si) const { 
        return vel_si * (SI_to_Engine_Length / SI_to_Engine_Time); 
    }
    
    double energySIToEngine(double energy_si) const {
        double E_scale = SI_to_Engine_Mass * (SI_to_Engine_Length / SI_to_Engine_Time) * (SI_to_Engine_Length / SI_to_Engine_Time);
        return energy_si * E_scale;
    }

    double toPixels(double engine_val) const { return engine_val * engine_to_pixels; }
    
    Vec2d coordsToPixels(const Vec2d& engine_pos) const {
        return Vec2d(engine_pos.x * engine_to_pixels, engine_pos.y * engine_to_pixels);
    }

    void setScreenScale(double pixels_per_unit) { engine_to_pixels = pixels_per_unit; }
};
