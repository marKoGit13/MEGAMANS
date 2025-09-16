#pragma once

struct Vec2 { // Vector 2D para posiciones y dimensiones
    float x;
    float y;
};

struct Rectangulo { // Rectángulo definido por su tamaño y posición central
    Vec2 anchoAlto; // x: ancho, y: alto
    Vec2 centerPosition; // Posición central (x, y)
};

bool ColisionRectangular(const Rectangulo& a, const Rectangulo& b); // Función para detectar colisiones entre dos rectángulos