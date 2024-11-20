/*
 *  Copyright 2022 <Claire Helms>
 */

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

class BothShader : public GShader
{
public:
    BothShader(GShader *s1, GShader *s2)
        : fs1(s1), fs2(s2) {}

    bool isOpaque() override
    {
        return fs1->isOpaque() && fs2->isOpaque();
    }

    bool setContext(const GMatrix &ctm) override
    {
        return fs1->setContext(ctm) && fs2->setContext(ctm);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override
    {
        // call both shaders and read data from them
        GPixel r0[count + 1];
        GPixel r1[count + 1];
        fs1->shadeRow(x, y, count, r0);
        fs2->shadeRow(x, y, count, r1);
        for (int i = 0; i < count; i++)
        {
            // multiply each color in the first shader by the corresponding color from the other shader
            // and divide to average it

            float a = ((GPixel_GetA(r0[i]) * GPixel_GetA(r1[i])) / 255.0f);
            float r = ((GPixel_GetR(r0[i]) * GPixel_GetR(r1[i])) / 255.0f);
            float g = ((GPixel_GetG(r0[i]) * GPixel_GetG(r1[i])) / 255.0f);
            float b = ((GPixel_GetB(r0[i]) * GPixel_GetB(r1[i])) / 255.0f);
            row[i] = GPixel_PackARGB(a, r, g, b);
        }
    }

private:
    GShader *fs1;
    GShader *fs2;
};