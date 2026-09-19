#version 400 core

// diamond.vert -- INM376 Final Coursework 2026
// Per-vertex Phong shading (Gouraud) for the CDiamond primitive
// and the 40 animated lane markers around the circuit.

uniform struct Matrices {
    mat4 projMatrix;
    mat4 modelViewMatrix;
    mat3 normalMatrix;
} matrices;

struct LightInfo {
    vec4 position;  // already in eye space
    vec3 La;
    vec3 Ld;
    vec3 Ls;
};
uniform LightInfo light1;

struct MaterialInfo {
    vec3  Ma;
    vec3  Md;
    vec3  Ms;
    float shininess;
};
uniform MaterialInfo material1;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inCoord;
layout(location = 2) in vec3 inNormal;

out vec3 vColour;
out vec2 vTexCoord;

vec3 PhongModel(vec4 eyePos, vec3 eyeNorm)
{
    vec3  s        = normalize(vec3(light1.position - eyePos));
    vec3  v        = normalize(-eyePos.xyz);
    vec3  r        = reflect(-s, eyeNorm);
    vec3  ambient  = light1.La * material1.Ma;
    float sDotN    = max(dot(s, eyeNorm), 0.0);
    vec3  diffuse  = light1.Ld * material1.Md * sDotN;
    vec3  specular = vec3(0.0);
    if (sDotN > 0.0)
        specular = light1.Ls * material1.Ms *
            pow(max(dot(r, v), 0.0), material1.shininess + 0.000001);
    return ambient + diffuse + specular;
}

void main()
{
    gl_Position = matrices.projMatrix
                * matrices.modelViewMatrix
                * vec4(inPosition, 1.0);

    vec4 eyePos  = matrices.modelViewMatrix * vec4(inPosition, 1.0);
    vec3 eyeNorm = normalize(matrices.normalMatrix * inNormal);

    vColour   = PhongModel(eyePos, eyeNorm);
    vTexCoord = inCoord;
}
