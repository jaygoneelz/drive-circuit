#include "Common.h"
#include "Diamond.h"
#define _USE_MATH_DEFINES
#include <math.h>

CDiamond::CDiamond()
    : m_vao(0), m_topVertCount(0), m_bottomVertCount(0)
{
}

CDiamond::~CDiamond()
{
}

void CDiamond::Create(string sTopTexture, string sBottomTexture)
{
    auto loadTex = [](CTexture& tex, const string& path)
        {
            tex.Load(path);
            tex.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            tex.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            tex.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_REPEAT);
            tex.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);
        };
    loadTex(m_topTexture, sTopTexture);
    loadTex(m_bottomTexture, sBottomTexture);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    m_vbo.Create();
    m_vbo.Bind();

    const int   N = 8;      // octagonal cross-section
    const float R = 1.0f;   // girdle ring radius
    const float TOP_Y = 1.5f;  // crown apex height
    const float BOT_Y = -1.0f;  // pavilion apex depth

    // Helper to push one vertex into the VBO
    auto push = [&](glm::vec3 pos, glm::vec2 uv, glm::vec3 n)
        {
            m_vbo.AddData(&pos, sizeof(glm::vec3));
            m_vbo.AddData(&uv, sizeof(glm::vec2));
            m_vbo.AddData(&n, sizeof(glm::vec3));
        };

    // Girdle ring vertices
    glm::vec3 G[8];
    for (int i = 0; i < N; i++)
    {
        float a = 2.0f * (float)M_PI * (float)i / (float)N;
        G[i] = glm::vec3(R * cosf(a), 0.0f, R * sinf(a));
    }

    glm::vec3 apex_top(0.0f, TOP_Y, 0.0f);
    glm::vec3 apex_bot(0.0f, BOT_Y, 0.0f);

    // Crown -- 8 triangles from top apex to each girdle edge
    for (int i = 0; i < N; i++)
    {
        glm::vec3 v0 = apex_top;
        glm::vec3 v1 = G[(i + 1) % N];
        glm::vec3 v2 = G[i];
        glm::vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        push(v0, glm::vec2(0.5f, 1.0f), n);
        push(v1, glm::vec2(1.0f, 0.0f), n);
        push(v2, glm::vec2(0.0f, 0.0f), n);
    }
    m_topVertCount = N * 3;

    // Pavilion -- 8 triangles from bottom apex to each girdle edge (reversed winding)
    for (int i = 0; i < N; i++)
    {
        glm::vec3 v0 = apex_bot;
        glm::vec3 v1 = G[i];
        glm::vec3 v2 = G[(i + 1) % N];
        glm::vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        push(v0, glm::vec2(0.5f, 0.0f), n);
        push(v1, glm::vec2(0.0f, 1.0f), n);
        push(v2, glm::vec2(1.0f, 1.0f), n);
    }
    m_bottomVertCount = N * 3;

    m_vbo.UploadDataToGPU(GL_STATIC_DRAW);

    // Interleaved: pos(12) | uv(8) | normal(12) = 32 bytes
    GLsizei stride = 2 * sizeof(glm::vec3) + sizeof(glm::vec2);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)sizeof(glm::vec3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
        (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void CDiamond::Render()
{
    glBindVertexArray(m_vao);
    m_topTexture.Bind();
    glDrawArrays(GL_TRIANGLES, 0, m_topVertCount);
    m_bottomTexture.Bind();
    glDrawArrays(GL_TRIANGLES, m_topVertCount, m_bottomVertCount);
}

void CDiamond::Release()
{
    m_topTexture.Release();
    m_bottomTexture.Release();
    glDeleteVertexArrays(1, &m_vao);
    m_vbo.Release();
}