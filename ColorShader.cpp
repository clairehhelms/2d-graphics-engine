#include "GShader.h"
#include "GPixel.h"
#include "GColor.h"
#include "GMatrix.h"
#include "GBitmap.h"
#include <iostream>
#include "GMath.h"
#include <algorithm>
#include <vector>
#include <stack>

class ColorShader : public GShader
{
public:
    ColorShader(GPoint P0, GPoint P1, GPoint P2, GColor C0, GColor C1, GColor C2)
    {
        fDC1 = C1 - C0;
        fDC2 = C2 - C0;
        fU = P1 - P0;
        fV = P2 - P0;
        fP0 = P0;
        fC0 = C0;
    }
    bool isOpaque() override { return false; }

    bool setContext(const GMatrix &ctm) override
    {
        fM = GMatrix(fU.x(), fV.x(), fP0.x(), fU.y(), fV.y(), fP0.y());

        return (ctm * fM).invert(&fInv);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override
    {

        // GColor DC = C[1] - C[0];
        for (int i = 0; i < count; i++)
        {
            GPoint rowpt = {(x + i + 0.5), (y + 0.5)};
            GPoint P = fInv * rowpt;
            GColor C = P.x() * fDC1 + P.y() * fDC2 + fC0;
            row[i] = makepixel(C);
        }

    }

    GPixel makepixel(GColor color)
    {
        float a = color.a * 255;
        float r = color.r * color.a * 255;
        float g = color.g * color.a * 255;
        float b = color.b * color.a * 255;
        a = std::max(std::min(a, 255.0f), 0.0f);
        r = std::max(std::min(r, a), 0.0f);
        g = std::max(std::min(g, a), 0.0f);
        b = std::max(std::min(b, a), 0.0f);
        return GPixel_PackARGB(a, r, g, b);
    }

private:
    GMatrix fCTM;
    GMatrix fM, fMinv;
    GMatrix fInv;
    GColor fDC1, fDC2, fDC, fC0;
    GPoint fU, fV, fP0;
};
