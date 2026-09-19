#pragma once

#include "Texture.h"
#include "VertexBufferObject.h"

// CDiamond
// Octagonal cut diamond built from 16 flat-shaded triangular faces:
// 8 upper crown faces and 8 lower pavilion faces.
// VBO layout: position(12) | uv(8) | normal(12) = 32 bytes per vertex.
// Two glDrawArrays calls are issued, one per texture half.

class CDiamond
{
public:
    CDiamond();
    ~CDiamond();

    void Create(string sTopTexture, string sBottomTexture);
    void Render();
    void Release();

private:
    UINT                m_vao;
    CVertexBufferObject m_vbo;
    CTexture            m_topTexture;
    CTexture            m_bottomTexture;
    int                 m_topVertCount;     // 24 (8 triangles x 3)
    int                 m_bottomVertCount;  // 24 (8 triangles x 3)
};