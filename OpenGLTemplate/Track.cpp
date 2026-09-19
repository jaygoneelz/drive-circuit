#include "Common.h"
#include "Track.h"

CTrack::CTrack()
    : m_vao(0), m_vertCount(0)
{
}

CTrack::~CTrack()
{
}

void CTrack::Create(CCatmullRom* spline, string sTexture,
    float halfWidth, float texRepeat)
{
    m_texture.Load(sTexture);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    m_texture.SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    m_vbo.Create();
    m_vbo.Bind();

    glm::vec3 normal(0.0f, 1.0f, 0.0f); // flat road, all normals point up
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    int n = spline->PathSize();

    for (int i = 0; i <= n; i++)
    {
        float t = (float)i / (float)n;
        glm::vec3 pos = spline->GetPosition(t);
        glm::vec3 T, N, B;
        spline->GetTNB(t, T, N, B);

        // Flat horizontal offset -- cross(worldUp, T) avoids NaN from near-vertical T
        glm::vec3 Nflat = glm::cross(worldUp, T);
        float len = glm::length(Nflat);
        Nflat = (len < 0.0001f) ? glm::vec3(1.0f, 0.0f, 0.0f) : Nflat / len;

        float y = 0.2f; // slightly above terrain; polygon offset handles Z-fighting
        float v = t * texRepeat;

        glm::vec3 left(pos.x + halfWidth * Nflat.x, y, pos.z + halfWidth * Nflat.z);
        glm::vec3 right(pos.x - halfWidth * Nflat.x, y, pos.z - halfWidth * Nflat.z);

        glm::vec2 uvL(0.0f, v), uvR(1.0f, v);

        m_vbo.AddData(&left, sizeof(glm::vec3));
        m_vbo.AddData(&uvL, sizeof(glm::vec2));
        m_vbo.AddData(&normal, sizeof(glm::vec3));

        m_vbo.AddData(&right, sizeof(glm::vec3));
        m_vbo.AddData(&uvR, sizeof(glm::vec2));
        m_vbo.AddData(&normal, sizeof(glm::vec3));
    }

    m_vertCount = (n + 1) * 2;
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

void CTrack::Render()
{
    glBindVertexArray(m_vao);
    m_texture.Bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertCount);
}

void CTrack::Release()
{
    m_texture.Release();
    glDeleteVertexArrays(1, &m_vao);
    m_vbo.Release();
}