#version 400 core

uniform struct Matrices {
    mat4 projMatrix;
    mat4 modelViewMatrix;
    mat3 normalMatrix;
} matrices;

struct LightInfo {
    vec4 position;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
};
uniform LightInfo light1;

struct SpotLightInfo {
    vec4  position;
    vec3  La;
    vec3  Ld;
    vec3  Ls;
    vec3  direction;
    float cutoff;
    float exponent;
};
uniform SpotLightInfo spotlight1;
uniform SpotLightInfo spotlight2;
uniform bool          bDarkMode;

struct MaterialInfo {
    vec3  Ma;
    vec3  Md;
    vec3  Ms;
    float shininess;
};
uniform MaterialInfo material1;

uniform float fogDensity;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inCoord;
layout(location = 2) in vec3 inNormal;

out vec3  vColour;
out vec2  vTexCoord;
out vec3  worldPosition;
out float vFogFactor;

// Phong shading for the directional sunlight
vec3 PhongModel(vec4 eyePos, vec3 eyeNorm)
{
    vec3 s = normalize(vec3(light1.position - eyePos));
    vec3 v = normalize(-eyePos.xyz);
    vec3 r = reflect(-s, eyeNorm);

    vec3  ambient  = light1.La * material1.Ma;
    float sDotN    = max(dot(s, eyeNorm), 0.0);
    vec3  diffuse  = light1.Ld * material1.Md * sDotN;
    vec3  specular = vec3(0.0);
    if (sDotN > 0.0)
        specular = light1.Ls * material1.Ms *
            pow(max(dot(r, v), 0.0), material1.shininess + 0.000001);

    return ambient + diffuse + specular;
}

// Phong shading for a spotlight with cone attenuation
vec3 SpotlightModel(SpotLightInfo sl, vec4 eyePos, vec3 eyeNorm)
{
    vec3  s        = normalize(vec3(sl.position - eyePos));
    float cosAngle = dot(-s, normalize(sl.direction));

    if (cosAngle < sl.cutoff)
        return sl.La * material1.Ma;

    float spotFactor = pow(cosAngle, sl.exponent);
    vec3  v          = normalize(-eyePos.xyz);
    vec3  r          = reflect(-s, eyeNorm);
    vec3  ambient    = sl.La * material1.Ma;
    float sDotN      = max(dot(s, eyeNorm), 0.0);
    vec3  diffuse    = sl.Ld * material1.Md * sDotN;
    vec3  specular   = vec3(0.0);
    if (sDotN > 0.0)
        specular = sl.Ls * material1.Ms *
            pow(max(dot(r, v), 0.0), material1.shininess + 0.000001);

    return ambient + spotFactor * (diffuse + specular);
}

void main()
{
    worldPosition = inPosition;

    gl_Position = matrices.projMatrix
                * matrices.modelViewMatrix
                * vec4(inPosition, 1.0);

    vec4 eyePos  = matrices.modelViewMatrix * vec4(inPosition, 1.0);
    vec3 eyeNorm = normalize(matrices.normalMatrix * inNormal);

    if (bDarkMode)
        vColour = SpotlightModel(spotlight1, eyePos, eyeNorm)
                + SpotlightModel(spotlight2, eyePos, eyeNorm);
    else
        vColour = PhongModel(eyePos, eyeNorm);

    vTexCoord = inCoord;

    // Exponential fog factor: 1.0 = no fog, 0.0 = fully fogged
    float dist = length(eyePos.xyz);
    vFogFactor = clamp(exp(-fogDensity * dist), 0.0, 1.0);
}
