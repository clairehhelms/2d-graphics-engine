/*
 *  Copyright 2022 <Claire Helms>
 */

#include <iostream>
#include <algorithm>
#include <vector>
#include <stack>
#include "GPath.h"
#include "GMatrix.h"

/**
 *  Append a new contour respecting the Direction. The contour should be an approximate
 *  circle (8 quadratic curves will suffice) with the specified center and radius.
 *
 *  Returns a reference to this path.
 */

GPath &GPath::addCircle(GPoint center, float radius, GPath::Direction direction)
{
    GPoint pts[16];

    float mpi= static_cast<float>(M_PI)/8;
    float sqrtt = static_cast<float>(sqrt(2))/2;
    pts[0] = {1, 0};
    pts[1] = {1, tan(mpi)};
    pts[2] = {sqrtt, sqrtt};
    pts[3] = {(mpi), 1};
    pts[4] = {0, 1};
    pts[5] = {(mpi), 1};
    pts[6] = {-sqrtt, sqrtt};
    pts[7] = {-1, tan(mpi)};
    pts[8] = {-1, 0};
    pts[9] = {-1, -tan(mpi)};
    pts[10] = {-sqrtt, -sqrtt};
    pts[11] = {-tan(mpi), -1};
    pts[12] = {0, -1};
    pts[13] = {tan(mpi), -1};
    pts[14] = {sqrtt, -sqrtt};
    pts[15] = {1, -tan(mpi)};

    GMatrix matrix = GMatrix::Concat(GMatrix::Translate(center.fX, center.fY), GMatrix::Scale(radius, radius));
    matrix.mapPoints(pts, 16);

    this->moveTo(pts[0]);
    for (int i = 0; i < 8; i++)
    {
        if (direction == GPath::kCCW_Direction)
        {
            this->quadTo(pts[i * 2 + 1], i != 7 ? pts[i * 2 + 2] : pts[0]);
        }
        else if (direction == GPath::kCW_Direction)
        {
            this->quadTo(pts[15 - i * 2], pts[14 - i * 2]);
        }
    }

    return *this;
}

// GPath &GPath::addCircle(GPoint center, float radius, Direction dir)
// {
//     // the 45 degree point on unit circle
//     float ffdeg = sqrt(2.0f) / 2.0f;
//     // tangent pi/8
//     float tpoe = tan(3.1415265f .0f);

//     float cubiclen = 0.551915;

//     // create the points of a circle before translation
//     // we have 8 curves, each quadrant has 2 curves, and each curve has 2 line segments
//     // the bisecting points are tangental in x and y to points 1 and 2
//     GPoint points[16];
//     GPoint mappedPoints[16];
//     // first quadrant
//     points[0] = {1, 0};
//     points[1] = {1, tpoe};

//     points[2] = {ffdeg, ffdeg};
//     points[3] = {tpoe, 1};

//     // Q2
//     points[4] = {0, 1};
//     points[5] = {-tpoe, 1};
//     points[6] = {-ffdeg, ffdeg};
//     points[7] = {-1, tpoe};

//     // Q3
//     points[8] = {-1, 0};
//     points[9] = {-1, -tpoe};
//     points[10] = {-ffdeg, -ffdeg};
//     points[11] = {-tpoe, -1};

//     // Q4
//     points[12] = {0, -1};
//     points[13] = {tpoe, -1};
//     points[14] = {tpoe, -ffdeg};
//     points[15] = {1, -tpoe};

//     GMatrix translated, translated1;
//     translated.Scale(radius, radius);
//     translated1.Translate(center.x(), center.y());
//     GMatrix mx = translated.Scale(radius, radius) * translated1.Translate(center.x(), center.y());
//     mx.mapPoints(mappedPoints, points, 16);
//     // now that we have mapped points, need to make the contour based on direction
//     this->moveTo(mappedPoints[0]);
//     if (dir == GPath::Direction::kCW_Direction)
//     {
//         // CW

//         this->quadTo(mappedPoints[1], mappedPoints[2]);
//         this->quadTo(mappedPoints[3], mappedPoints[4]);
//         this->quadTo(mappedPoints[5], mappedPoints[6]);
//         this->quadTo(mappedPoints[7], mappedPoints[8]);
//         this->quadTo(mappedPoints[9], mappedPoints[10]);
//         this->quadTo(mappedPoints[11], mappedPoints[12]);
//         this->quadTo(mappedPoints[13], mappedPoints[14]);
//         this->quadTo(mappedPoints[15], mappedPoints[0]);

//     }
//     else if (dir == GPath::Direction::kCCW_Direction)
//     {
//         // CCW

//         this->quadTo(mappedPoints[15], mappedPoints[14]);
//         this->quadTo(mappedPoints[13], mappedPoints[12]);
//         this->quadTo(mappedPoints[11], mappedPoints[10]);
//         this->quadTo(mappedPoints[9], mappedPoints[8]);
//         this->quadTo(mappedPoints[7], mappedPoints[6]);
//         this->quadTo(mappedPoints[5], mappedPoints[4]);
//         this->quadTo(mappedPoints[3], mappedPoints[2]);
//         this->quadTo(mappedPoints[1], mappedPoints[0]);

//     }

//     return *this;
// }

/**
 *  Given 0 < t < 1, subdivide the src[] quadratic bezier at t into two new quadratics in dst[]
 *  such that
 *  0...t is stored in dst[0..2]
 *  t...1 is stored in dst[2..4]
 */
void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t)
{
    // dest and src are the same at 0
    dst[0] = src[0];
    // the last entries are also the same!
    dst[4] = src[2];
    // the rest are calculated in a similar way to the linear gradient ...
    dst[1] = ((1 - t) * src[0]) + (t * src[1]);
    dst[3] = ((1 - t) * src[1]) + (t * src[2]);
    // but this one is tricky because it's between the newly calc'd dst points
    dst[2] = ((1 - t) * dst[1]) + (t * dst[3]);
}

/**
 *  Given 0 < t < 1, subdivide the src[] cubic bezier at t into two new cubics in dst[]
 *  such that
 *  0...t is stored in dst[0..3]
 *  t...1 is stored in dst[3..6]
 */
void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t)
{
    // dst and source same at 0
    dst[0] = src[0];
    // last entries also same
    // note: i think the allocated arrays are too big lmao
    dst[6] = src[3];
    // rest calc'd:
    dst[1] = ((1 - t) * src[0]) + (t * src[1]);
    dst[2] = (1 - t) * dst[1] + t * ((1 - t) * src[1] + t * src[2]);
    dst[5] = (1 - t) * src[2] + t * src[3];
    dst[4] = (1 - t) * ((1 - t) * src[1] + t * src[2]) + t * dst[5];
    dst[3] = (1 - t) * dst[2] + t * dst[4];
}

GPath &GPath::addRect(const GRect &r, Direction dir)
{
    // starting at top left, add rect to path based on direction
    GIRect rr = r.round();
    int L = rr.left();
    int R = rr.right();
    int T = rr.top();
    int B = rr.bottom();

    // top left:
    this->moveTo({L, T});

    // CW
    if (dir == 0)
    {
        this->lineTo({R, T});
        this->lineTo({R, B});
        this->lineTo({L, B});
        this->lineTo({L, T});
    }
    else if (dir == 1)
    {
        // CCW
        this->lineTo({L, B});
        this->lineTo({R, B});
        this->lineTo({R, T});
        this->lineTo({L, T});
    }
    return *this;
}

GPath &GPath::addPolygon(const GPoint pts[], int count)
{
    if (count < 3)
    {
        return *this;
    }
    this->moveTo(pts[0]);
    for (int i = 1; i < count; i++)
    {
        this->lineTo(pts[i]);
    }
    // this->lineTo(pts[0]);
    return *this;
}

GRect GPath::bounds() const
{
    float L = 1000;
    float R = 0;
    float T = 1000;
    float B = 0;
    if (this->countPoints() == 0)
    {
        return GRect::LTRB(0, 0, 0, 0);
    }
    for (GPoint p : this->fPts)
    {
        L = std::min(L, p.fX);
        R = std::max(R, p.fX);
        T = std::min(T, p.fY);
        B = std::max(B, p.fY);
    }
    return GRect::LTRB(L, T, R, B);
};

void GPath::transform(const GMatrix &m)
{
    m.mapPoints(&(this->fPts[0]), &(this->fPts[0]), this->countPoints());
}
