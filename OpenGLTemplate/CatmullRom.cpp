#include "Common.h"
#include "CatmullRom.h"
#define _USE_MATH_DEFINES
#include <math.h>

CCatmullRom::CCatmullRom()
    : m_totalLength(0.0f)
{
}

CCatmullRom::~CCatmullRom()
{
}

void CCatmullRom::AddControlPoint(glm::vec3 p)
{
    m_controlPoints.push_back(p);
}

// Standard Catmull-Rom basis with tension 0.5 (C1 continuity at joints)
glm::vec3 CCatmullRom::Interpolate(glm::vec3 p0, glm::vec3 p1,
    glm::vec3 p2, glm::vec3 p3, float t) const
{
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5f * (
        (2.0f * p1) +
        (-p0 + p2) * t +
        (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
        (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
        );
}

void CCatmullRom::CreatePath(int stepsPerSegment)
{
    m_centrelinePoints.clear();
    m_arcLengths.clear();
    m_totalLength = 0.0f;

    int n = (int)m_controlPoints.size();
    if (n < 2) return;

    for (int i = 0; i < n; i++)
    {
        glm::vec3 p0 = m_controlPoints[(i - 1 + n) % n];
        glm::vec3 p1 = m_controlPoints[i];
        glm::vec3 p2 = m_controlPoints[(i + 1) % n];
        glm::vec3 p3 = m_controlPoints[(i + 2) % n];

        for (int s = 0; s < stepsPerSegment; s++)
        {
            float localT = (float)s / (float)stepsPerSegment;
            glm::vec3 pt = Interpolate(p0, p1, p2, p3, localT);
            m_centrelinePoints.push_back(pt);

            if (m_centrelinePoints.size() > 1)
                m_totalLength += glm::length(pt - m_centrelinePoints[m_centrelinePoints.size() - 2]);

            m_arcLengths.push_back(m_totalLength);
        }
    }

    // Close the loop
    if (!m_centrelinePoints.empty())
        m_totalLength += glm::length(m_centrelinePoints.front() - m_centrelinePoints.back());
}

// Binary search for the sample closest to arc-length t * totalLength
int CCatmullRom::ArcLengthToIndex(float t) const
{
    if (m_arcLengths.empty()) return 0;
    float target = t * m_totalLength;
    int lo = 0, hi = (int)m_arcLengths.size() - 1;
    while (lo < hi)
    {
        int mid = (lo + hi) / 2;
        if (m_arcLengths[mid] < target) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

glm::vec3 CCatmullRom::GetPosition(float t) const
{
    if (m_centrelinePoints.empty()) return glm::vec3(0.0f);
    t = t - floorf(t);
    int idx = ArcLengthToIndex(t);
    return m_centrelinePoints[idx % (int)m_centrelinePoints.size()];
}

void CCatmullRom::GetTNB(float t, glm::vec3& T, glm::vec3& N, glm::vec3& B) const
{
    if (m_centrelinePoints.size() < 2)
    {
        T = glm::vec3(1, 0, 0);
        N = glm::vec3(0, 0, 1);
        B = glm::vec3(0, 1, 0);
        return;
    }

    t = t - floorf(t);
    int n = (int)m_centrelinePoints.size();
    int idx = ArcLengthToIndex(t);
    int prev = (idx - 1 + n) % n;
    int next = (idx + 1) % n;

    // Central difference for tangent
    T = glm::normalize(m_centrelinePoints[next] - m_centrelinePoints[prev]);

    // Binormal from T x worldUp, normal from B x T
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    B = glm::normalize(glm::cross(T, worldUp));
    N = glm::normalize(glm::cross(B, T));
}

void CCatmullRom::GetEdgePoints(float t, float halfWidth,
    glm::vec3& leftPt, glm::vec3& rightPt) const
{
    glm::vec3 pos = GetPosition(t);
    glm::vec3 T, N, B;
    GetTNB(t, T, N, B);
    leftPt = pos + halfWidth * N;
    rightPt = pos - halfWidth * N;
}