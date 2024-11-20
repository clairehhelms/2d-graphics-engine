/*
 *  Copyright 2022 <Claire Helms>
 */

#include "GMatrix.h"
#include "GBitmap.h"
#include <iostream>
#include "GMath.h"
#include <algorithm>
#include <vector>
#include <stack>

GMatrix::GMatrix()
{
    this->fMat[0] = 1;
    this->fMat[1] = 0;
    this->fMat[2] = 0;
    this->fMat[3] = 0;
    this->fMat[4] = 1;
    this->fMat[5] = 0;
}

GMatrix GMatrix::Translate(float tx, float ty)
{
    // 3rd column of matrix is used to translate/offset values
    return GMatrix(1, 0, tx, 0, 1, ty);
}

GMatrix GMatrix::Scale(float sx, float sy)
{
    // sx and sy live at x and y's respective multipier mx indices
    return GMatrix(sx, 0, 0, 0, sy, 0);
}

GMatrix GMatrix::Rotate(float rads)
{
    // cos, -sin, sin, cos ... describes rotation for x and y
    // looks like cos and sin are built in??
    return GMatrix(cos(rads), -1 * sin(rads), 0, sin(rads), cos(rads), 0);
}


GMatrix GMatrix::Concat(const GMatrix &secundo, const GMatrix &primo)
{
    // given two matrices, return a new multiplied one!
    // oh it's actually not that deep, multiply each index
    float a = secundo[0] * primo[0] + secundo[1] * primo[3];
    float b = secundo[0] * primo[1] + secundo[1] * primo[4];
    float c = secundo[0] * primo[2] + secundo[1] * primo[5] + secundo[2];
    float d = secundo[3] * primo[0] + secundo[4] * primo[3];
    float e = secundo[3] * primo[1] + secundo[4] * primo[4];
    float f = secundo[3] * primo[2] + secundo[4] * primo[5] + secundo[5];
    return GMatrix(a, b, c, d, e, f);
}

bool GMatrix::invert(GMatrix *inverse) const
{
    // refactor to use this
    float a = this->fMat[0];
    float b = this->fMat[1];
    float c = this->fMat[2];
    float d = this->fMat[3];
    float e = this->fMat[4];
    float f = this->fMat[5];
    float det = (a * e) - (b * d);
    if (det == 0)
    {
        return false;
    }

    // else...
    det = 1.0 / det;
    *inverse = GMatrix(det * e, -det * b, det * (b * f - e * c), -det * d, det * a, det * (d * c - a * f));
    return true;
}

/**
 *  Transform the set of points in src, storing the resulting points in dst, by applying this
 *  matrix. It is the caller's responsibility to allocate dst to be at least as large as src.
 */
void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const
{
    for (int i = 0; i < count; i++)
    {
        float srcx = src[i].fX;
        float srcy = src[i].fY;

        // x' = ax + by + c
        float xp = this->fMat[0] * srcx + this->fMat[1] * srcy + this->fMat[2];
        float yp = this->fMat[3] * srcx + this->fMat[4] * srcy + this->fMat[5];

        dst[i] = GPoint({xp, yp});
    }
}
