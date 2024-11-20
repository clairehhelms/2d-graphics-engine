/*
 *  Copyright 2022 <Claire Helms>
 */

#include "GShader.h"
#include "GMatrix.h"
#include "GBitmap.h"
#include <iostream>
#include "GMath.h"
#include <algorithm>
#include <vector>
#include <stack>

class CShader : public GShader
{
public:
    CShader(const GBitmap &bm, const GMatrix &lm, GShader::TileMode tm) : fBM(bm), fLM(lm), fTm(tm){};

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
    void shadeRow(int x, int y, int count, GPixel row[]) override
    {
        GMatrix *invptr = &fInv;

        // loop count
        for (int i = 0; i < count; i++)
        {
            // transform, floor, pin, locate
            // crerate a point for the current pixel center
            GPoint rowpt = {(x + i + 0.5), (y + 0.5)};
            GPoint mappedpt;
            // run that point thru the inv mx
            invptr->mapPoints(&mappedpt, &rowpt, 1);
            float widthFract = 1.0f / fBM.width();
            float heightFract = 1.0f / fBM.height();

            // floor the resulting x and y
            int ix, iy;
            // ix = mappedpt.fX/fBM.width();
            // iy = mappedpt.fY/fBM.height();

            // get into 0,1 space
            float fix = mappedpt.fX * widthFract;
            float fiy = mappedpt.fY * heightFract;

            if (fTm == GShader::TileMode::kClamp)
            {
                // clamp the resulting x and y
                fix = std::min(std::max(fix, 0.0f), 1.0f);
                fiy = std::min(std::max(fiy, 0.0f), 1.0f);
            }
            else if (fTm == GShader::TileMode::kMirror)
            {
                fix *= 0.5;
                fiy *= 0.5;

                fix -= floor(fix);
                fiy -= floor(fiy);

                fix > 0.5 ? fix = 1.0f - fix : fix = fix;
                fiy > 0.5 ? fiy = 1.0f - fiy : fiy = fiy;
                fix *= 2;
                fiy *= 2;
            }
            else if (fTm == GShader::TileMode::kRepeat)
            { 
                fix = fix - GFloorToInt(fix);
                fiy = fiy - GFloorToInt(fiy);
            }
            // return to bitmap coords
            ix = GFloorToInt(fix * (fBM.width() - 1));
            iy = GFloorToInt(fiy * (fBM.height() - 1));
            ix = std::min(std::max(ix, 0), fBM.width());
            iy = std::min(std::max(iy, 0), fBM.height());

            // get pixel at original space's x and y

            row[i] = *fBM.getAddr(ix, iy);
            
        }
    }

private:
    GBitmap fBM;
    GMatrix fLM;
    GMatrix fInv;
    GMatrix fCTM;
    GShader::TileMode fTm;
};

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap &bm, const GMatrix &localMatrix, GShader::TileMode tm)
{
    return std::unique_ptr<GShader>(new CShader(bm, localMatrix, tm));
}
