#include <iostream>
#include "raylib.h"

void rysowanie_hit_box(Rectangle hitbox, Color kolor)
{
    DrawRectangleLines( // rysowanie hit boxów bo je trzeba lekko poprawiać
        (int)hitbox.x,
        (int)hitbox.y,
        (int)hitbox.width,
        (int)hitbox.height,
        kolor);
}

static bool TooCloseX(int x, float otherX, int minDist)
{
    return std::abs(x - (int)otherX) < minDist;
}

static bool TooCloseForSzkielet(int x, int minDist,
                                const Vector2 &duchPos,
                                const Vector2 &batPos,
                                const Vector2 &szczurPos)
{
    return TooCloseX(x, duchPos.x, minDist) ||
           TooCloseX(x, batPos.x, minDist) ||
           TooCloseX(x, szczurPos.x, minDist);
}

static bool TooCloseForDuch(int x, int minDist,
                            const Vector2 &szkieletPos,
                            const Vector2 &batPos,
                            const Vector2 &szczurPos)
{
    return TooCloseX(x, szkieletPos.x, minDist) ||
           TooCloseX(x, batPos.x, minDist) ||
           TooCloseX(x, szczurPos.x, minDist);
}

static bool TooCloseForBat(int x, int minDist,
                           const Vector2 &duchPos,
                           const Vector2 &szkieletPos,
                           const Vector2 &szczurPos)
{
    return TooCloseX(x, duchPos.x, minDist) ||
           TooCloseX(x, szkieletPos.x, minDist) ||
           TooCloseX(x, szczurPos.x, minDist);
}

static bool TooCloseForSzczur(int x, int minDist,
                              const Vector2 &duchPos,
                              const Vector2 &szkieletPos,
                              const Vector2 &batPos)
{
    return TooCloseX(x, duchPos.x, minDist) ||
           TooCloseX(x, szkieletPos.x, minDist) ||
           TooCloseX(x, batPos.x, minDist);
}