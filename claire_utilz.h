/*
 *  Copyright 2022 <Claire Helms>
 */

#include "GCanvas.h"
#include "GRect.h"
#include "GColor.h"
#include "GBitmap.h"
#include "GPixel.h"
#include "GPath.h"
#include <iostream>
#include "GMath.h"
#include <algorithm>
#include <vector>

struct Edge
{
    float fCurrX;
    float fSlope;
    int fY;
    int fLastY;
    int fWind = -1; // if p0 < p1

    bool init(GPoint p0, GPoint p1)
    {
        assert((p0.fY >= 0) && (p1.fY >= 0));
        if (p0.fY > p1.fY)
        {
            fWind = 1; // going down
            std::swap(p0, p1);
        }
        int y0 = GRoundToInt(p0.fY);
        int y1 = GRoundToInt(p1.fY);
        if (y0 == y1)
        {
            // do we ever check this?
            return false;
        }
        fSlope = (p1.fX - p0.fX) / (p1.fY - p0.fY);
        fCurrX = p0.fX + fSlope * (p1.fY - p0.fY + 0.5f);
        fY = GRoundToInt(y0);
        fLastY = GRoundToInt(p1.fY - 1);
        if (fY == fLastY) {
            return false;
        }
        return true;
    }
};

unsigned div255(unsigned value)
{
    const unsigned K = 65793;
    return (value * K + (1 << 23)) >> 24;
};

GPixel srcOver(GPixel src, GPixel dest)
{
    // assuming premul colors
    // int base = 255;
    // divides are deathspell to CPU...
    // S + (1 - Sa)*D
    int sa = GPixel_GetA(src);
    int sr = GPixel_GetR(src);
    int sg = GPixel_GetG(src);
    int sb = GPixel_GetB(src);
    int da = GPixel_GetA(dest);
    int dr = GPixel_GetR(dest);
    int dg = GPixel_GetG(dest);
    int db = GPixel_GetB(dest);
    int norm = 255 - sa;

    unsigned a = sa + div255(norm * da);
    unsigned r = sr + div255(norm * dr);
    unsigned g = sg + div255(norm * dg);
    unsigned b = sb + div255(norm * db);
    return GPixel_PackARGB(a, r, g, b);
}

GPixel srcIn(GPixel src, GPixel dst)
{
    // Da * S
    int da = GPixel_GetA(dst);
    int sa = GPixel_GetA(src);
    int sr = GPixel_GetR(src);
    int sg = GPixel_GetG(src);
    int sb = GPixel_GetB(src);
    unsigned a = div255(da * sa);
    unsigned r = div255(da * sr);
    unsigned g = div255(da * sg);
    unsigned b = div255(da * sb);
    return GPixel_PackARGB(a, r, g, b);
}

GPixel srcOut(GPixel src, GPixel dst)
{
    // (1 - Da)*S
    int dax = 255 - GPixel_GetA(dst);
    int sa = GPixel_GetA(src);
    int sr = GPixel_GetR(src);
    int sg = GPixel_GetG(src);
    int sb = GPixel_GetB(src);

    unsigned a = div255(dax * sa);
    unsigned r = div255(dax * sr);
    unsigned g = div255(dax * sg);
    unsigned b = div255(dax * sb);
    return GPixel_PackARGB(a, r, g, b);
}

GPixel srcATop(GPixel src, GPixel dst)
{
    // Da*S + (1 - Sa)*D
    int sa = GPixel_GetA(src);
    int sr = GPixel_GetR(src);
    int sg = GPixel_GetG(src);
    int sb = GPixel_GetB(src);
    int da = GPixel_GetA(dst);
    int dr = GPixel_GetR(dst);
    int dg = GPixel_GetG(dst);
    int db = GPixel_GetB(dst);

    unsigned a = div255(da * sa) + div255((255 - sa) * da);
    unsigned r = div255(da * sr) + div255((255 - sa) * dr);
    unsigned g = div255(da * sg) + div255((255 - sa) * dg);
    unsigned b = div255(da * sb) + div255((255 - sa) * db);
    return GPixel_PackARGB(a, r, g, b);
}

GPixel sdXor(GPixel src, GPixel dst)
{
    // (1 - Sa)*D + (1 - Da)*S
    int sa = GPixel_GetA(src);
    int sr = GPixel_GetR(src);
    int sg = GPixel_GetG(src);
    int sb = GPixel_GetB(src);
    int da = GPixel_GetA(dst);
    int dr = GPixel_GetR(dst);
    int dg = GPixel_GetG(dst);
    int db = GPixel_GetB(dst);

    unsigned a = div255((255 - sa) * da) + div255((255 - da) * sa);
    unsigned r = div255((255 - sa) * dr) + div255((255 - da) * sr);
    unsigned g = div255((255 - sa) * dg) + div255((255 - da) * sg);
    unsigned b = div255((255 - sa) * db) + div255((255 - da) * sb);
    return GPixel_PackARGB(a, r, g, b);
}

GPixel colorToPixel(GColor color)
{
    float a = color.a * 255;
    float r = color.r * color.a * 255;
    float g = color.g * color.a * 255;
    float b = color.b * color.a * 255;
    return GPixel_PackARGB(a, r, g, b);
}

GPixel blendme(GBlendMode blmd, GPixel dst, GPixel src)
{
    // TODO: add shortcuts for when srca is 1 or 0
    switch (blmd)
    {
    case GBlendMode::kClear:
    {
        return 0;
    };
    case GBlendMode::kSrc:
    {
        return src;
    };
    case GBlendMode::kDst:
    {
        return dst;
    };
    case GBlendMode::kSrcOver:
    {
        return srcOver(src, dst);
    };
    case GBlendMode::kDstOver:
    {
        return srcOver(dst, src);
    };
    case GBlendMode::kSrcIn:
    {
        return srcIn(src, dst);
    };
    case GBlendMode::kDstIn:
    {
        return srcIn(dst, src);
    };
    case GBlendMode::kSrcOut:
    {
        return srcOut(src, dst);
    };
    case GBlendMode::kDstOut:
    {
        return srcOut(dst, src);
    };
    case GBlendMode::kSrcATop:
    {
        return srcATop(src, dst);
    };
    case GBlendMode::kDstATop:
    {
        return srcATop(dst, src);
    };
    case GBlendMode::kXor:
    {
        return sdXor(src, dst);
    };
    default:
        break;
    }
}

GPoint eval_cubic(const GPoint pts[3], float t)
{
    GPoint A = (pts[3] - pts[0]) + 3.0f * (pts[1] - pts[2]);
    GPoint B = 3.0f * ((pts[2] - pts[1]) + (pts[0] - pts[1]));
    GPoint C = 3.0f * (pts[1] - pts[0]);
    GPoint D = pts[0];
    return ((A * t + B) * t + C) * t + D;
}

GPoint eval_quad(const GPoint pts[], float t)
{
    // std::cout << pts[0].x() << std::endl;
    GPoint A = pts[0] + -2.0f * pts[1] + pts[2];
    GPoint B = 2.0f * (pts[1] + (-1.0f) * (pts[0]));
    GPoint C = pts[0];
    // std::cout << ((A * t + B) * t + C).x() << std::endl;
    return (A * t + B) * t + C;
}

int segCount(GPath::Verb v, GPoint pts[])
{
    // tolerance is 1/4 of a pixel, therefore
    float tol = 0.25f;

    // error term 

    if (v == GPath::kQuad)
    {
        GPoint A = pts[0] + (-2) * pts[1] + pts[2];
        float E = sqrt(A.x() * A.x() + A.y() * A.y());
        return GCeilToInt(sqrt(abs(E) / tol));
    }
    else if (v == GPath::kCubic)
    {
        // abc = a - 2b + c
        // bcd = b - 2c + d

        // GPoint abc = pts[0] - pts[1] - pts[1] + pts[2];
        // GPoint bcd = pts[1] - pts[2] - pts[2] + pts[3];
        // float dx = std::max(std::abs(abc.fX), std::abs(bcd.fX));
        // float dy = std::max(std::abs(abc.fY), std::abs(bcd.fY));
        // float E0 = sqrt(abc.x() * abc.x() + abc.y() * abc.y());
        // float E1 = sqrt(bcd.x() * B.x() + B.y() * B.y());
        // return GCeilToInt(sqrt(0.75 *std::max(E0, E1) / tol));

        GPoint A = pts[0] + (-2) * pts[1] + pts[2];
        GPoint B = pts[1] + (-2) * pts[2] + pts[3];

        float E0 = sqrt(A.x() * A.x() + A.y() * A.y());
        float E1 = sqrt(B.x() * B.x() + B.y() * B.y());
        GPoint E;

        return GCeilToInt(sqrt(0.75 * std::max(E0, E1) / tol));
    }
    printf("SHOUDL NOT BE HERE\n");
    return 0;
}