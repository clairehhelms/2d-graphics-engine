/*
 *  Copyright 2022 <Claire Helms>
 */

#include "GShader.h"
#include "GMatrix.h"
#include "GBitmap.h"
#include "GColor.h"
#include <iostream>
#include "GMath.h"
#include <algorithm>
#include <vector>
#include <stack>

class CGradient : public GShader
{
public:
    CGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode tm)
    {
        for (int i = 0; i < count; i++)
        {
            fColors.push_back(colors[i]);
        }
        fCount = count;
        if (p0.x() < p1.x())
        {
            fP0 = p0;
            fP1 = p1;
        }
        else
        {
            fP0 = p1;
            fP1 = p0;
        }

        float p0x = fP0.x();
        float p0y = fP0.y();
        float p1x = fP1.x();
        float p1y = fP1.y();

        // mx = dx, -dy, p0.x, dy, dx, p0.y
        fLM = GMatrix(p1x - p0x, -(p1y - p0y), p0x, p1y - p0y, p1x - p0x, p0y);

        fTm = tm;
    }

    bool isOpaque() override
    {
        return false;
    }

    bool setContext(const GMatrix &ctm) override
    {
        // updates fCTM, returns false if it's not invertible
        this->fCTM = ctm;
        // took this line from MR's code, mults & inverts the mxs both for bool return and future shadeRow use
        return (ctm * fLM).invert(&fInv);
    }

    // for a given row, find the color on the gradient at that pixel
    void shadeRow(int x, int y, int count, GPixel row[]) override
    {
        GMatrix *invptr = &fInv;
        // transform, floor, pin, locate
        // create a point for the current pixel center
        GPoint rowpt = {(x + 0.5), (y + 0.5)};
        GPoint mappedpt;
        // run that point thru the inv mx
        invptr->mapPoints(&mappedpt, &rowpt, 1);
        float mX = mappedpt.fX;
        // loop count (the row)
        for (int i = 0; i < count; i++)
        {
            float ix, fx;
            // clamp in (0, 1) space
            if (fTm == GShader::TileMode::kClamp)
            {
                // clamp the resulting x and y
                ix = std::min(std::max(mX, 0.0f), 1.0f);
            }
            else if (fTm == GShader::TileMode::kMirror)
            {
                // get into 0,1 space
                ix = mX;
                ix *= 0.5;

                ix = ix - floor(ix);
               
                ix > 0.5 ? ix = 1 - ix : ix = ix;
                ix *= 2;
            }
            else if (fTm == GShader::TileMode::kRepeat)
            { // get into 0,1 coordinates
                ix = mX;
                ix = ix - GFloorToInt(ix);
            }

            fx = ix * (fCount - 1);


            // float ix = std::min(std::max(mX, 0.0f), 1.0f);

            // find color at the current loc and make it a pixel, store it in row[]
            GColor mC;
            // first color will be current index and next will be next idx

            // first, find origin pt on line
            int idx = GFloorToInt(fx);

            // then, find the colors that correspond
            GColor c0 = fColors[idx];
            GColor c1 = fColors[idx + 1];

            // compute the ratio'd location in (0, 1) space
            float w = fx - idx;

            // finally, make color and store
            mC = {
                c0.r + (w * (c1.r - c0.r)),
                c0.g + (w * (c1.g - c0.g)),
                c0.b + (w * (c1.b - c0.b)),
                c0.a + (w * (c1.a - c0.a))};

            row[i] = colorToPixel(mC);
            
            // only incrementing x by a
            mX = fInv[0] + mX;
        }
    }

private:
    std::vector<GColor> fColors;
    GMatrix fLM;
    GMatrix fInv;
    GMatrix fCTM;
    GPoint fP0;
    GPoint fP1;
    int fCount;
    GShader::TileMode fTm;

    GPixel colorToPixel(GColor color)
    {
        float a = color.a * 255;
        float r = color.r * color.a * 255;
        float g = color.g * color.a * 255;
        float b = color.b * color.a * 255;
        return GPixel_PackARGB(a, r, g, b);
    }
};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode tm)
{
    return std::unique_ptr<GShader>(new CGradient(p1, p0, colors, count, tm));
}