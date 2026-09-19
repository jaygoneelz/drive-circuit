#pragma once

#include "Texture.h"
#include "VertexBufferObject.h"
#include "CatmullRom.h"

// CTrack
// Generates a flat ribbon road along a Catmull-Rom spline as a GL_TRIANGLE_STRIP.
// Interleaved VBO layout: position(12) | uv(8) | normal(12) = 32 bytes per vertex,
// matching the CDiamond layout so both use the same vertex attribute setup.

class CTrack
{
public:
    CTrack();
    ~CTrack();

    // spline     -- centreline (CreatePath must have been called first)
    // sTexture   -- road texture file path
    // halfWidth  -- half the road width in world units
    // texRepeat  -- texture tile count along the full circuit length
    void Create(CCatmullRom* spline, string sTexture,
        float halfWidth = 8.0f, float texRepeat = 50.0f);

    void Render();
    void Release();

private:
    UINT                m_vao;
    CVertexBufferObject m_vbo;
    CTexture            m_texture;
    int                 m_vertCount;
};