#pragma once
#include "Common.h"

// CCatmullRom
// Builds a closed Catmull-Rom spline from a list of control points.
// After CreatePath() has been called, provides uniform arc-length
// parameterisation, world positions, and TNB frames at any t in [0,1].

class CCatmullRom
{
public:
    CCatmullRom();
    ~CCatmullRom();

    void AddControlPoint(glm::vec3 p);

    // Sample the spline. Must be called after all control points are added.
    void CreatePath(int stepsPerSegment = 50);

    int PathSize() const { return (int)m_centrelinePoints.size(); }

    // World position at normalised arc-length t
    glm::vec3 GetPosition(float t) const;

    // TNB frame at t -- T=forward, N=left (XZ plane), B=up binormal
    void GetTNB(float t, glm::vec3& T, glm::vec3& N, glm::vec3& B) const;

    // Left and right edge positions at t offset by halfWidth
    void GetEdgePoints(float t, float halfWidth,
        glm::vec3& leftPt, glm::vec3& rightPt) const;

    const std::vector<glm::vec3>& ControlPoints()    const { return m_controlPoints; }
    const std::vector<glm::vec3>& CentrelinePoints() const { return m_centrelinePoints; }

private:
    glm::vec3 Interpolate(glm::vec3 p0, glm::vec3 p1,
        glm::vec3 p2, glm::vec3 p3, float t) const;

    int ArcLengthToIndex(float t) const;

    std::vector<glm::vec3> m_controlPoints;
    std::vector<glm::vec3> m_centrelinePoints;
    std::vector<float>     m_arcLengths;
    float                  m_totalLength;
};