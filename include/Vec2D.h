#ifndef VEC2D_H
#define VEC2D_H

#include <cmath>

struct Vec2d
{
    double x{};
    double y{};
    
    Vec2d() = default;
    Vec2d(const Vec2d&) = default;
    Vec2d(double xx, double yy): x{xx}, y{yy} 
    {}

    Vec2d operator-() const {
        return Vec2d(-x, -y);
    }
    Vec2d& operator = (const Vec2d&) = default;

    Vec2d& operator += (const Vec2d& v){
        x += v.x;
        y += v.y;
        return *this;
    }

    Vec2d& operator -= (const Vec2d& v){
        x -= v.x;
        y -= v.y;
        return *this;
    }

    Vec2d& operator *= (double v){
        x *= v;
        y *= v;
        return *this;
    }

    Vec2d& operator /= (double v){
        x /= v;
        y /= v;
        return *this;
    }
};

inline Vec2d operator + (Vec2d v1, const Vec2d& v2) { return v1 += v2; }
inline Vec2d operator - (Vec2d v1, const Vec2d& v2) { return v1 -= v2; }
inline Vec2d operator * (Vec2d v1, double d) { return v1 *= d; }
inline Vec2d operator / (Vec2d v1, double d) { return v1 /= d; }    
    
inline double dot(const Vec2d& v1, const Vec2d& v2){
    return v1.x * v2.x + v1.y * v2.y;
}

inline double leight(const Vec2d& v){
    return std::sqrt(dot(v, v));
}

inline Vec2d rotate(double angle, const Vec2d& v){
    double cosa{std::cos(angle)};
    double sina{std::sin(angle)};
    return Vec2d{ v.x * cosa - v.y * sina,
                  v.x * sina + v.y * cosa};
}
#endif