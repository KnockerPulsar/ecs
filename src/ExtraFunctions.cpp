#pragma once
#include "raylib.h"

bool Between(float less, float value, float more)
{
    return (less < value) && (value < more);
}
bool BetweenEq(float less, float value, float more)
{
    return (less <= value) && (value <= more);
}


void XCHG(float &A, float &B)
{
    float temp = A;
    A = B;
    B = temp;
}

// For text allignment
void DrawCross()
{
    for (int i = 0; i < GetScreenWidth(); i++)
    {
        DrawPixel(i, GetScreenHeight() / 2, GREEN);
    }
    for (int i = 0; i < GetScreenHeight(); i++)
    {
        DrawPixel(GetScreenWidth() / 2, i, GREEN);
    }
}

Vector2 Vector2Multiply(Vector2 a, float scalar)
{
    return Vector2{a.x * scalar, a.y * scalar};
}
