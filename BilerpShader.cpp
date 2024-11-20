#include "GShader.h"
#include "GMatrix.h"
#include "GBitmap.h"
#include <iostream>
#include "GMath.h"
#include <algorithm>
#include <vector>
#include <stack>

class BilerpShader : public GShader
{
public:
    BilerpShader(const GBitmap &bm, const GMatrix &lm) : fBM(bm), fLM(lm){};

    bool isOpaque() override
    {
        return this->fBM.isOpaque();
    }

    bool setContext(const GMatrix &ctm) override
    {
        // updates fCTM, returns false if it's not invertible
        this->fCTM = ctm;
        // took this line from MR's code, mults & inverts the mxs both for bool return and future shadeRow use
        return (ctm * fLM).invert(&fInv);
    }

    /**
     *  Given a row of pixels in device space [x, y] ... [x + count - 1, y], return the
     *  corresponding src pixels in row[0...count - 1]. The caller must ensure that row[]
     *  can hold at least [count] entries.
     */

    // p0, p1
    // p2, p3
    // p0.x = GFloorToInt(x - 0.5f);
    // p0.y = GFloorToInt(y - 0.5f);
    // p1.x = GFloorToInt(x + 0.5f);
    // p1.y = GFloorToInt(y - 0.5f);
    // p2.x = GFloorToInt(x - 0.5f);
    // p2.y = GFloorToInt(y + 0.5f);
    // p3.x = GFloorToInt(x + 0.5f);
    // p3.y = GFloorToInt(y + 0.5f);

    void shadeRow(int x, int y, int count, GPixel row[]) override
    {
        GMatrix *invptr = &fInv;

        // loop count
        for (int i = 0; i < count; i++)
        {
            // transform, floor, pin, locate
            // crerate a point for the current pixel center
            GPoint p0, p1, p2, p3;

            GPoint rowpt = {(x + i + 0.5), (y + 0.5)};
            GPoint mappedpt;
            // run that point thru the inv mx
            invptr->mapPoints(&mappedpt, &rowpt, 1);
            float widthFract = 1.0f / fBM.width();
            float heightFract = 1.0f / fBM.height();

            // floor the resulting x and y
            float ix, iy;
            ix = mappedpt.fX;
            iy = mappedpt.fY;
            p0.fX = std::min(std::max(GFloorToInt(ix - 0.5f), 0), fBM.width() - 1);
            p0.fY = std::min(std::max(GFloorToInt(iy - 0.5f), 0), fBM.height() - 1);
            p1.fX = std::min(std::max(GFloorToInt(ix + 0.5f), 0), fBM.width() - 1);
            p1.fY = std::min(std::max(GFloorToInt(iy - 0.5f), 0), fBM.height() - 1);
            p2.fX = std::min(std::max(GFloorToInt(ix - 0.5f), 0), fBM.width() - 1);
            p2.fY = std::min(std::max(GFloorToInt(iy + 0.5f), 0), fBM.height() - 1);
            p3.fX = std::min(std::max(GFloorToInt(ix + 0.5f), 0), fBM.width() - 1);
            p3.fY = std::min(std::max(GFloorToInt(iy + 0.5f), 0), fBM.height() - 1);

            if (GFloorToInt(ix) != 0 && GFloorToInt(ix) < fBM.width() - 1)
            {
                if(p0.fX == p1.fX) {
                    p1.fX += 1;
                }
            }

            // ix = std::min(std::max(x, 0), fBM.width());
            // iy = std::min(std::max(y, 0), fBM.height());

            // get colors at p0-p3
            GPixel c0, c1, c2, c3;
            c0 = *fBM.getAddr(p0.fX, p0.fY);
            c1 = *fBM.getAddr(p1.fX, p1.fY);
            c2 = *fBM.getAddr(p2.fX, p2.fY);
            c3 = *fBM.getAddr(p3.fX, p3.fY);

            float u = (ix - p0.fX) / (p1.fX - p0.fX);
            float v = (iy - p0.fY) / (p2.fY - p0.fY);

            float newp_a = (1.0f - u) * (1.0f - v) * GPixel_GetA(c0) + u * (1.0f - v) * GPixel_GetA(c1) + v * (1.0f - u) * GPixel_GetA(c2) + u * v * GPixel_GetA(c3);
            float newp_r = (1.0f - u) * (1.0f - v) * GPixel_GetR(c0) + u * (1.0f - v) * GPixel_GetR(c1) + v * (1.0f - u) * GPixel_GetR(c2) + u * v * GPixel_GetR(c3);
            float newp_g = (1.0f - u) * (1.0f - v) * GPixel_GetG(c0) + u * (1.0f - v) * GPixel_GetG(c1) + v * (1.0f - u) * GPixel_GetG(c2) + u * v * GPixel_GetG(c3);
            float newp_b = (1.0f - u) * (1.0f - v) * GPixel_GetB(c0) + u * (1.0f - v) * GPixel_GetB(c1) + v * (1.0f - u) * GPixel_GetB(c2) + u * v * GPixel_GetB(c3);

            newp_a = std::min(newp_a, 255.0f);
            newp_r = std::min(newp_a, newp_r);
            newp_g = std::min(newp_a, newp_g);
            newp_b = std::min(newp_a, newp_b);
            GPixel newp = GPixel_PackARGB(newp_a, newp_r, newp_g, newp_b);
            row[i] = newp;
        }
    }

private:
    GBitmap fBM;
    GMatrix fLM;
    GMatrix fInv;
    GMatrix fCTM;
    GShader::TileMode fTm;
};

// std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap &bm, const GMatrix &localMatrix)
// {
//     return std::unique_ptr<GShader>(new BilerpShader(bm, localMatrix));
// }
