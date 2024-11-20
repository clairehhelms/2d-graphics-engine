/*
 *  Copyright 2022 <Claire Helms>
 */

#include "GCanvas.h"
#include "GRect.h"
#include "GColor.h"
#include "GBitmap.h"
#include "GPixel.h"
#include <iostream>
#include "GMath.h"
#include <algorithm>
#include <vector>

void clip(GPoint &P0, GPoint &P1, std::vector<Edge> &edges, GBitmap device)
{
    int w = device.width();
    int h = device.height();

    // if ys are same, we can ignore
    if (P0.fY == P1.fY)
    {
        return;
    }
    // if all points are inside, just make the edge
    if (P0.fY >= 0 && P0.fX >= 0 && P0.fX < w && P0.fY < h && P1.fY >= 0 && P1.fX >= 0 && P1.fX < w && P1.fY < h)
    {
        Edge e;

        // e.init(P0, P1);
        if (e.init(P0, P1))
        {
            edges.push_back(e);
        }
    }
    else
    {
        GPoint bottom;
        GPoint top;

        // assigning top and bottom to make my life easier.
        if (P0.fY > P1.fY)
        {
            bottom = P0;
            top = P1;
        }
        else
        {
            bottom = P1;
            top = P0;
        }

        // if point's y is smaller than 0, no need for new edges, just new points
        if (top.fY < 0)
        {
            // if both are less than 0, may ignore <3
            if (bottom.fY < 0)
            {
                return;
            }
            if (top.fX == bottom.fX)
            {
                top.fY = 0;
            }
            else
            {
                // else, compute new top vals
                top.fX = top.fX + (bottom.fX - top.fX) * (-top.fY) / (bottom.fY - top.fY);
                top.fY = 0;
            }
        }
        // if point's y is larger than h, no need for new edges, just new points
        if (bottom.fY > h)
        {
            if (top.fY > h)
            {
                return;
            }
            if (top.fX == bottom.fX)
            {
                bottom.fY = h - 1;
            }
            else
            {
                bottom.fX = bottom.fX - (bottom.fX - top.fX) * ((bottom.fY - (h)) / (bottom.fY - top.fY));
                bottom.fY = h - 1;
            }
        }

        GPoint left;
        GPoint right;
        // swap top and bottom if necessary
        if (top.fX > bottom.fX)
        {
            left = bottom;
            right = top;
        }
        else
        {
            right = bottom;
            left = top;
        }
        // now checking left and right bounds
        // first, if both left and right are out of bounds...
        if (right.fX < 0 && left.fX < 0)
        {
            right.fX = 0;
            left.fX = 0;
        }
        if (right.fX >= w && left.fX >= w)
        {
            right.fX = w - 1;
            left.fX = w - 1;
        }
        // check left
        if (left.fX < 0)
        {
            GPoint QP;
            QP.fX = 0;
            QP.fY = left.fY;
            left.fY = left.fY + (right.fY - left.fY) * (-left.fX) / (right.fX - left.fX);
            left.fX = 0;
            Edge e;
            if (e.init(QP, left))
            {
                edges.push_back(e);
            }
        }
        // check right
        if (right.fX > w)
        {
            GPoint QP;
            QP.fX = w - 1;
            QP.fY = right.fY;
            right.fY = right.fY - (right.fY - left.fY) * (right.fX - w) / (right.fX - left.fX);
            right.fX = w - 1;
            Edge e;
            if (e.init(QP, right)) {
                edges.push_back(e);
            };
        }
        Edge e;
        if (e.init(left, right))
        {
            edges.push_back(e);
        };
    }
}
