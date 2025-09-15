#include "colisiones.h"

bool ColisionRectangular(const Rectangulo& a, const Rectangulo& b) {
    float aHalfW = a.anchoAlto.x / 2.0f;
    float aHalfH = a.anchoAlto.y / 2.0f;
    float bHalfW = b.anchoAlto.x / 2.0f;
    float bHalfH = b.anchoAlto.y / 2.0f;

    float aLeft = a.centerPosition.x - aHalfW;
    float aRight = a.centerPosition.x + aHalfW;
    float aTop = a.centerPosition.y - aHalfH;
    float aBottom = a.centerPosition.y + aHalfH;

    float bLeft = b.centerPosition.x - bHalfW;
    float bRight = b.centerPosition.x + bHalfW;
    float bTop = b.centerPosition.y - bHalfH;
    float bBottom = b.centerPosition.y + bHalfH;

    if (aRight < bLeft || aLeft > bRight || aBottom < bTop || aTop > bBottom) {
        return false;
    }
    return true;
}