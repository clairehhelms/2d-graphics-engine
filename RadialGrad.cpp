#include "GShader.h"
#include "GMatrix.h"
#include "GBitmap.h"
#include "GColor.h"
#include <iostream>
#include "GMath.h"
#include <algorithm>
#include <vector>
#include <stack>

class RadialGrad : public GShader
{
public:
    RadialGrad(GPoint center, const GColor colors[], int count, float radius, GShader::TileMode tm)
    {
        for (int i = 0; i < count; i++)
        {
            fColors.push_back(colors[i]);
        }
        fCount = count;
        fCenter = center;
        fRadius = radius;

        // Scale
        fLM.Translate(fCenter.x(), fCenter.y());

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
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GMatrix *invptr = &fInv;
        // transform, floor, pin, locate
        // create a point for the current pixel center
        GPoint rowpt = {(x + 0.5), (y + 0.5)};
        GPoint mappedpt;
        // run that point thru the inv mx
        invptr->mapPoints(&mappedpt, &rowpt, 1);

        float mX = mappedpt.fX;
        float mY = mappedpt.fY;

        float centerY = fCenter.y();
        float centerX = fCenter.x();
        float ydiff = abs(mY - centerY);
        float ysquared = pow(ydiff, 2);
        float xdiff = abs(mX - centerX);
        // loop count (the row)
        for (int i = 0; i < count; i++)
        {
            float xsquared = pow(xdiff, 2);
            float l = sqrt(xsquared + ysquared);
            float ratio = (float)l/(float)fRadius;

            float ix, fx;

            if (fTm == GShader::TileMode::kClamp)
            {
                // clamp the resulting x and y
                ix = std::min(std::max(ratio, 0.0f), 1.0f);
            }
            else if (fTm == GShader::TileMode::kMirror)
            {
                // get into 0,1 space
                ix = ratio;
                ix *= 0.5;

                ix = ix - floor(ix);
               
                ix > 0.5 ? ix = 1 - ix : ix = ix;
                ix *= 2;
            }
            else if (fTm == GShader::TileMode::kRepeat)
            { // get into 0,1 coordinates
                ix = ratio;
                ix = ix - GFloorToInt(ix);
            }


            // fx is finding the corresponding colpr
            // and then putting it in idx
            fx = ix * (fCount - 1);
            int idx = GFloorToInt(fx);

            // then, find the colors that correspond
            GColor c0 = fColors[idx];
            GColor c1 = fColors[idx + 1];

            // compute the ratio'd location in (0, 1) space
            float w = fx - idx;

            GColor mC;
            // finally, make color and store
            mC = {
                c0.r + (w * (c1.r - c0.r)),
                c0.g + (w * (c1.g - c0.g)),
                c0.b + (w * (c1.b - c0.b)),
                c0.a + (w * (c1.a - c0.a))};

            row[i] = ctoP(mC);
            
            // only incrementing x by a
            mX = fInv[0] + mX;
            xdiff--;
    }
    }

    GPixel ctoP(GColor color)
    {
        float a = color.a * 255;
        float r = color.r * color.a * 255;
        float g = color.g * color.a * 255;
        float b = color.b * color.a * 255;
        return GPixel_PackARGB(a, r, g, b);
    }

private:
    std::vector<GColor> fColors;
    GMatrix fLM;
    GMatrix fInv;
    GMatrix fCTM;
    GPoint fCenter;
    float fRadius;
    int fCount;
    GShader::TileMode fTm;
};

// std::unique_ptr<GShader> GCreateLinearGradient(GPoint center, GColor colors[], int count, float radius, GShader::TileMode mode)
// {
//     return std::unique_ptr<GShader>(new RadialGrad(center, colors, count, radius, mode));
// }