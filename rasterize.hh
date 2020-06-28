//
// Created by letba on 6/28/2020.
//

#ifndef RENDERER_RASTERIZE_HH
#define RENDERER_RASTERIZE_HH

#include <type_traits>
#include <utility>
#include <tuple>
#include <algorithm>

template<typename P>
void RasterizeTriangle
        (const P *p0, const P *p1, const P *p2,
         auto &&GetXY,
         auto &&MakeSlope,
         auto &&DrawScanline)requires std::invocable<decltype(GetXY), const P &>
                                      and std::invocable<decltype(MakeSlope), const P *, const P *, int>
                                      and (std::tuple_size_v<std::remove_cvref_t<decltype(GetXY(*p0))>> == 2)
                                      and requires {{ +std::get<0>(GetXY(*p0)) } -> std::integral; }
                                      and requires {{ +std::get<1>(GetXY(*p0)) } -> std::integral; }
                                      and requires(std::remove_cvref_t<decltype(MakeSlope(p0, p1, 1))> a) {
    DrawScanline(1, a, a);
} {
    auto[x0, y0, x1, y1, x2, y2] = std::tuple_cat(GetXY(*p0), GetXY(*p1), GetXY(*p2));

    if (std::tie(y1, x1) < std::tie(y0, x0)) {
        std::swap(x0, x1);
        std::swap(y0, y1);
        std::swap(p0, p1);
    }
    if (std::tie(y2, x2) < std::tie(y0, x0)) {
        std::swap(x0, x2);
        std::swap(y0, y2);
        std::swap(p0, p2);
    }
    if (std::tie(y2, x2) < std::tie(y1, x1)) {
        std::swap(x1, x2);
        std::swap(y1, y2);
        std::swap(p1, p2);
    }

    // Refuse to draw arealess triangles.
    if (int(y0) == int(y2)) return;

    // Determine whether the short side is on the left or on the right.
    bool shortside = (y1 - y0) * (x2 - x0) < (x1 - x0) * (y2 - y0); // false=left side, true=right side

    // Create two slopes: p0-p1 (short) and p0-p2 (long).
    // One of these is on the left side and one is on the right side.
    // At y = y1, the p0-p1 slope will be replaced with p1-p2 slope.
    // The type of these variables is whatever MakeSlope() returns.
    std::invoke_result_t<decltype(MakeSlope), P *, P *, int> sides[2];

    // At this point, y2-y0 cannot be zero.
    sides[!shortside] = MakeSlope(p0, p2, y2 - y0); // Slope for long side

    // The main rasterization loop.
    /*
    if(int(y0) < int(y1))
    {
        // Calculate the first slope for short side. The number of lines cannot be zero.
        sides[shortside] = MakeSlope(p0,p1, y1-y0);
        for(int y = y0; y < int(y1); ++y)
        {
            // On a single scanline, we go from the left X coordinate to the right X coordinate.
            DrawScanline(y, sides[0], sides[1]);
        }
    }
    if(int(y1) < int(y2))
    {
        // Calculate the second slope for short side. The number of lines cannot be zero.
        sides[shortside] = MakeSlope(p1,p2, y2-y1);
        for(int y = y1; y < int(y2); ++y)
        {
            // On a single scanline, we go from the left X coordinate to the right X coordinate.
            DrawScanline(y, sides[0], sides[1]);
        }
    }
    */
    // The main rasterizing loop. Note that this is intentionally designed such that
    // there's only one place where DrawScanline() is invoked. This will minimize the
    // chances that the compiler fails to inline the functor.
    for (auto y = y0, endy = y0;; ++y) {
        if (y >= endy) {
            // If y of p2 is reached, the triangle is complete.
            if (y >= y2) break;
            // Recalculate slope for short side. The number of lines cannot be zero.
            sides[shortside] = std::apply(MakeSlope, (y < y1) ? std::tuple(p0, p1, (endy = y1) - y0)
                                                              : std::tuple(p1, p2, (endy = y2) - y1));
        }
        // On a single scanline, we go from the left X coordinate to the right X coordinate.
        DrawScanline(y, sides[0], sides[1]);
    }
}

#endif //RENDERER_RASTERIZE_HH
