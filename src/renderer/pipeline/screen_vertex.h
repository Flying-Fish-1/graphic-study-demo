#ifndef RENDERER_PIPELINE_SCREEN_VERTEX_H
#define RENDERER_PIPELINE_SCREEN_VERTEX_H

#include "geometry_stage.h"

namespace Renderer {
namespace Pipeline {

struct ScreenVertex {
    GeometryVertex attributes;
    float screenX;
    float screenY;
    float ndcZ;
    bool valid = false;
};

struct RasterDerivatives {
    float dudx;
    float dudy;
    float dvdx;
    float dvdy;
};

} // namespace Pipeline
} // namespace Renderer

#endif // RENDERER_PIPELINE_SCREEN_VERTEX_H
