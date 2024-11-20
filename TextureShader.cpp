#include "GShader.h"
#include "GMatrix.h"
#include "GBitmap.h"
#include <iostream>
#include "GMath.h"
#include <algorithm>
#include <vector>
#include <stack>

class TextureShader : public GShader
{
public:
    TextureShader(GShader *shader, const GMatrix &proxyMx) : fRealShader(shader), fProxyMx(proxyMx){};

    bool isOpaque() override
    {
        return false;
    };

    bool setContext(const GMatrix &ctm) override
    {
        GMatrix concatted;
        concatted = GMatrix::Concat(ctm, fProxyMx);
        return fRealShader->setContext(concatted);
    };

    void shadeRow(int x, int y, int count, GPixel row[]) override
    {
        fRealShader->shadeRow(x, y, count, row);
    };
    


private:
    GShader *fRealShader;
    GMatrix fProxyMx;
};