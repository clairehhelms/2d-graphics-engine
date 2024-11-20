/*
 *  Copyright 2022 <Claire Helms>
 */

#include "GCanvas.h"
#include "GPoint.h"
#include "GPath.h"
#include "GShader.h"
#include "GMatrix.h"
#include "GRect.h"
#include "GColor.h"
#include "GBitmap.h"
#include "GPixel.h"
#include "claire_utilz.h"
#include "clip.h"
#include <iostream>
#include "GMath.h"
#include <algorithm>
#include <vector>

class MyCanvas : public GCanvas
{
public:
    MyCanvas(const GBitmap &device) : fDevice(device)
    {
        GMatrix starter_mx = GMatrix();
        stack.push_back(starter_mx);
    }

    virtual void drawPath(const GPath &path, const GPaint &paint) override
    {
        if (path.countPoints() < 3)
        {
            return;
        }

        // create a copy of the path to use so this worrks
        GPath pathcpy = path;
        pathcpy.transform(stack.back());
        // edger makes our edges <3
        GPath::Edger edger(pathcpy);
        GPath::Verb v;
        GPoint pts[4];
        std::vector<Edge> edges = {};
        // clipping each edge from the edger
        while ((v = edger.next(pts)) != GPath::kDone)
        {
            // must check for quad, cubic, or line in here
            // in case there is a path with multiple types
            if (v == GPath::kLine)
            {
                // ctm -> mapints(points, points, 2)]]
                clip(pts[0], pts[1], edges, fDevice);
                if (edges.size() > 0)
                {
                    assert(edges.back().fY >= 0);
                }
            }
            else
            {
                // getting the amount of segments and where they will lay in (0, 1) coordinates
                int segmentCount = segCount(v, pts);
                // int segmentCount = 3;
                // printf("seg count: \n");
                // std::cout << segmentCount << std::endl;
                float step = 1.0f / segmentCount;
                GPoint P1, P2;
                GPoint (*evalfn)(const GPoint *pts, float t);
                if (v == GPath::kQuad)
                {
                    evalfn = &eval_quad;
                }
                else
                {
                    evalfn = &eval_cubic;
                }
                // for (float t = 0.0f; t < 1.0f; t += step)
                // stick to the beginning and end here
                // P1 = pts[0];
                // P2 = evalfn(pts, 0.0f);
                // at the end, p2 = pts[1]
                // for (int i = 0; i < segmentCount; i++)
                // if (v == GPath::kCubic) {
                //     Gpoint a = map
                // }
                GPoint save = P1;
                
                for (float t = 0.0f; t < 1.0f; t += step)
                {
                    // WE MUST MAKE THE LINES TOUCH
                    // the previous p2 is the new p1
                    // here is where we maintain topological contract 

                    P1 = P2;
                    P2 = evalfn(pts, t);
                    if (t == 0.0f)
                    {
                        clip(pts[0], P2, edges, fDevice);
                    }
                    else
                    {
                        clip(P1, P2, edges, fDevice);
                    }
                    if (edges.size() > 0)
                    {
                        assert(edges.back().fY >= 0);
                    }
                }
                if (v == GPath::kQuad)
                {
                    clip(P2, pts[2], edges, fDevice);
                }
                else
                {
                    clip(P2, pts[3], edges, fDevice);
                }
                // clip(P2, pts[2], edges, fDevice);
            }
        }
        // clip(pts[0], pts[1], edges, fDevice);

        // sort (y, x, m)
        if (edges.size() == 0)
        {
            return;
        }
        // sort by y
        std::sort(edges.begin(), edges.end(), pred);
        assert(edges[0].fY >= 0);
        // scan -> blit
        complex_scan(edges, edges.size(), paint);
    }

    void complex_scan(std::vector<Edge> &edges, int count, const GPaint &paint)
    {
        if (count <= 0)
        {
            return;
        }

        assert(edges[0].fY >= 0);
        for (int y = edges[0].fY; count > 0 && y < fDevice.height(); y++)
        {
            int i = 0; // index
            int w = 0; // wind tracker
            Edge L, R; // left and right edge
            // assert(false);

            // loop through active edges
            while ((i < count) && (edges[i].fY <= y))
            {
                assert(edges[i].fY >= 0);
                // resort = false;
                if (i >= edges.size())
                {
                    return;
                }
                if (w == 0)
                {
                    L = edges[i];
                }
                w += edges[i].fWind;
                if (w == 0)
                {
                    R = edges[i];
                    if (R.fCurrX < L.fCurrX) {
                        Edge tmp = L;
                        L = R;
                        R = tmp;
                    }
                    blit(L, R, y, paint);
                }
                if ((y - 1) >= edges[i].fLastY)
                {
                    edges.erase(edges.begin() + i);
                    count--;
                }
                else
                {
                    edges[i].fCurrX += edges[i].fSlope;
                    i++;
                }
                std::sort(edges.begin(), edges.begin() + i, sortByX);
            }
            // assert(w == 0);
        }
    }

    void blit(Edge &L, Edge &R, int y, GPaint paint)
    {
        // settin' up
        int Lx = GRoundToInt(L.fCurrX);
        int Rx = GRoundToInt(R.fCurrX);

        GColor color = paint.getColor();
        GPixel src = colorToPixel(color);
        GBlendMode blmd = paint.getBlendMode();

        // if there is a shader
        if (paint.getShader() != nullptr)
        {
            // if the currrent matrix is valid
            if (paint.getShader()->setContext(stack.back()))
            {
                // creating row and shading said row
                GPixel row[Rx - Lx];
                paint.getShader()->shadeRow(Lx, y, Rx - Lx, row);

                // doing the real coloring
                for (int x = Lx; x < Rx && x >= 0 && x < fDevice.width(); x++)
                {
                    GPixel *dst = fDevice.getAddr(x, y);
                    // if opaque, assign to pixel in row ("src"). else, blend
                    if (paint.getShader()->isOpaque())
                    {
                        *dst = row[x - Lx];
                    }
                    else
                    {
                        *dst = blendme(blmd, *dst, row[x - Lx]);
                    }
                }
            }
            // we're done
            return;
        }
        else
        {
            // elif there's not a shader, simple blit
            for (int x = Lx; x < Rx && x >= 0 && x < fDevice.width(); x++)
            {
                GPixel *p = fDevice.getAddr(x, y);
                if (blmd == GBlendMode::kSrcOver)
                {
                    int sa = GPixel_GetA(src);
                    int sr = GPixel_GetR(src);
                    int sg = GPixel_GetG(src);
                    int sb = GPixel_GetB(src);
                    int da = GPixel_GetA(*p);
                    int dr = GPixel_GetR(*p);
                    int dg = GPixel_GetG(*p);
                    int db = GPixel_GetB(*p);
                    int norm = 255 - sa;

                    unsigned a = sa + div255(norm * da);
                    unsigned r = sr + div255(norm * dr);
                    unsigned g = sg + div255(norm * dg);
                    unsigned b = sb + div255(norm * db);
                    *p = GPixel_PackARGB(a, r, g, b);
                }
                else
                {
                    *p = blendme(blmd, *p, src);
                }
            }
        }
    }

    virtual void save() override
    {
        // push a copy of the topmost matrix on the stack./
        // GMatrix CTM = stack.back();
        stack.push_back(stack.back());
    }
    virtual void restore() override
    {
        // pop last elem from stack
        if (stack.size() > 0)
        {
            stack.pop_back();
        }
        else
        {
            throw 505;
        }
    }
    virtual void concat(const GMatrix &mx) override
    {
        // add stored mx + incoming mx + mutate og mx
        GMatrix CTM = stack.back();
        restore();
        stack.push_back(CTM * mx);
    }

    void drawPaint(const GPaint &paint) override
    {
        GBlendMode blmd = paint.getBlendMode();
        GColor color = paint.getColor();
        GPixel src = colorToPixel(color);
        // height and width of bitmap
        int h = fDevice.height();
        int w = fDevice.width();

        // set the CTM for the shader if there is one
        if (paint.getShader() != nullptr)
        {
            if (paint.getShader()->setContext(stack.back()))
            {
                for (int y = 0; y < h; y++)
                {
                    GPixel row[w];
                    paint.getShader()->shadeRow(0, y, w, row);
                    for (int x = 0; x < w; x++)
                    {
                        GPixel *dst = fDevice.getAddr(x, y);
                        *dst = blendme(blmd, *dst, row[x]);
                    }
                }
            }
            // we're done
            return;
        }
        for (int y = 0; y < h; y++)
        {
            for (int x = 0; x < w; x++)
            {
                GPixel *p = fDevice.getAddr(x, y);
                *p = blendme(blmd, *p, src);
            };
        };
    }

    void drawRect(const GRect &rect, const GPaint &paint) override
    {
        // checks and stores params of rectangle, comparing against boundaries of bm
        GIRect rr = rect.round();
        int L = rr.left();
        int R = rr.right();
        int T = rr.top();
        int B = rr.bottom();

        // if rect has a shader, let's please convert to a 4-sided polygon
        if (paint.getShader() != nullptr)
        {
            GPoint P0 = {L, T};
            GPoint P1 = {L, B};
            GPoint P2 = {R, B};
            GPoint P3 = {R, T};
            GPoint pts[] = {P0, P1, P2, P3};
            drawConvexPolygon(pts, 4, paint);
            return;
        };

        int DW = fDevice.width();
        int DH = fDevice.height();

        int left = L >= 0 ? L : 0;
        int right = R <= DW ? R : DW;
        int top = T >= 0 ? T : 0;
        int bottom = B <= DH ? B : DH;

        // check source's alpha first
        GColor color = paint.getColor();
        GPixel src = colorToPixel(color);
        GBlendMode blmd = paint.getBlendMode();
        for (int y = top; y < bottom; y++)
        {
            for (int x = left; x < right; x++)
            {
                GPixel *p = fDevice.getAddr(x, y);
                *p = blendme(blmd, *p, src);
            }
        }
    }

    void drawConvexPolygon(const GPoint pts[], int count, const GPaint &paint) override
    {
        if (count < 3)
        {
            return;
        }

        // translate the points to the ones we need thru the CTM (returns same if no mx)
        GPoint mapped_pts[count];
        stack.back().mapPoints(mapped_pts, pts, count);

        // build edges (the order of the points are the order of the connections)
        std::vector<Edge> edges = {};

        // for each point in pts array, create edge between and push to vec
        for (int i = 0; i < count - 1; i++)
        {
            GPoint P0 = mapped_pts[i];
            GPoint P1 = mapped_pts[i + 1];
            clip(P0, P1, edges, fDevice);
        };
        // connect final edge
        GPoint P0 = mapped_pts[count - 1];
        GPoint P1 = mapped_pts[0];

        clip(P0, P1, edges, fDevice);

        // sort (y, x, m)
        std::sort(edges.begin(), edges.end(), pred);

        int top = 0;
        int bottom = fDevice.height();
        if (edges.size() != 0)
        {
            top = edges[0].fY;
            bottom = edges.back().fLastY;
        }
        else
        {
            return;
        }

        // loop-y
        int i = 0;
        int j = 1;

        // store the current L and R edges
        Edge L = edges[i];
        Edge R = edges[j];

        // set the CTM for the shader if there is one
        if (paint.getShader() != nullptr)
        {
            paint.getShader()->setContext(stack.back());
        }

        // loop y, set L and R
        for (int y = top; (y <= bottom) && (y >= 0) && (y < fDevice.height()); y++)
        {
            if (GRoundToInt(y - 1) >= edges[i].fLastY)
            {
                i = (std::max(i, j) + 1);
                L = edges[i];
            }
            if (GRoundToInt(y - 1) >= edges[j].fLastY)
            {
                j = (std::max(i, j) + 1);
                R = edges[j];
            }
            if (i >= edges.size() || j >= edges.size())
            {
                // we're done
                return;
            }

            // call the fn that traverses the x on the row we're on
            blit(L, R, y, paint);
            // update x vals
            L.fCurrX = L.fSlope + L.fCurrX;
            R.fCurrX = R.fSlope + R.fCurrX;
        }
    }
    

    // comparison tool for edge sorting
    static bool pred(Edge const &e1, Edge const &e2)
    {
        if (e1.fY != e2.fY)
        {
            return e1.fY < e2.fY;
        }
        if (e1.fCurrX != e2.fCurrX)
        {
            return e1.fCurrX < e2.fCurrX;
        }
        return e1.fSlope < e2.fSlope;
    }

    static bool sortByX(Edge const &e1, Edge const &e2)
    {
        if (e1.fCurrX != e2.fCurrX)
        {
            return e1.fCurrX < e2.fCurrX;
        }
        return e1.fSlope < e2.fSlope;
    }

private:
    // Note: we store a copy of the bitmap
    const GBitmap fDevice;
    std::vector<GMatrix> stack;
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap &device)
{
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}
