
#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
using namespace std;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
float magnitude(float x[2]);
float dotProduct(float x[2], float y[2]);
float* difference(float x[2], float y[2]);
float* normalize(float x[2]);
float* scale(float x[2], float s);
float AngleBetweenVectors(float* vector1, float* vector2, bool direction);
// Dot product with another vector.
float dotProduct(float x[2], float y[2]) {
    float dot = 0;
    for (int i = 0; i < 2; i++)
    {
        dot += x[i] * y[i];
    }
    return dot;
}
// Get the magnitude of this vector.
float magnitude(float x[2]) 
{
    return sqrt(dotProduct(x, x));
}
// Difference with another vector.
float* difference(float x[2], float y[2])
{
    int i;
    float* diff = new float[2];
    for (i = 0; i < 2; i++) diff[i] = x[i] - y[i];
    return diff;
}
// Get a copy of this vector multiplied by a scalar.
float* scale(float x[2], float s) 
{
    float* scaledX = new float[2];
    for (int i = 0; i < 2; i++)  scaledX[i] = s * x[i];
    return scaledX;
}
// Get normalized copy of this vector
float* normalize(float x[2]) 
{
    float m = magnitude(x);
    if (m == 0) return x;
    return scale(x, 1 / m);
}
// Compute the angle between two vectors. If direction matters, then negative 
// angles are those that are counterclockwise from first to second vector
float AngleBetweenVectors(float* vector1, float* vector2, bool direction)
{
    float* v1norm = normalize(vector1);
    float* v2norm = normalize(vector2);
    float dot = dotProduct(v1norm, v2norm);
    float angle = 180 * acos(dot) / M_PI;
    if (direction)
        if (v1norm[0] * v2norm[1] - v1norm[1] * v2norm[0] > 0) angle *= -1;
    return angle;
}