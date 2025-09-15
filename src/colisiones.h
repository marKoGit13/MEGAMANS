#pragma once

struct Vec2 {
    float x;
    float y;
};

struct Rectangulo {
    Vec2 anchoAlto; // x: ancho, y: alto
    Vec2 centerPosition;
};

bool ColisionRectangular(const Rectangulo& a, const Rectangulo& b);