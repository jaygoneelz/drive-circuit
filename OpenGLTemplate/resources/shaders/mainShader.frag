#version 400 core

in vec3  vColour;
in vec2  vTexCoord;
in vec3  worldPosition;
in float vFogFactor;

out vec4 vOutputColour;

uniform sampler2D   sampler0;
uniform sampler2D   sampler1;
uniform samplerCube CubeMapTex;
uniform bool  bUseTexture;
uniform bool  renderSkybox;
uniform bool  bMultiTex;
uniform vec3  fogColour;
uniform bool  bStormMode;
uniform float uTime;

void main()
{
    if (renderSkybox) {
        vOutputColour = texture(CubeMapTex, worldPosition);
        return;
    }

    // Texture sampling
    vec4 texColour;
    if (bMultiTex) {
        // Multi-texturing: base road texture blended with a finer detail overlay
        vec4 t0 = texture(sampler0, vTexCoord);
        vec4 t1 = texture(sampler1, vTexCoord * 4.0);
        texColour = mix(t0, t1, 0.35);
    } else {
        texColour = texture(sampler0, vTexCoord);
    }

    vec4 litColour = bUseTexture
        ? texColour * vec4(vColour, 1.0)
        : vec4(vColour, 1.0);

    // Exponential distance fog
    vec3 finalRGB = mix(fogColour, litColour.rgb, vFogFactor);

    if (bStormMode) {
        // Rain streaks -- three overlapping layers at different angles and speeds
        float r1u = fract(vTexCoord.x * 40.0 + vTexCoord.y * 2.0 + uTime * 0.1);
        float r1v = fract(vTexCoord.y * 8.0  - uTime * 12.0);
        float s1  = step(0.96,  r1u) * clamp(1.0 - r1v * 2.0, 0.0, 1.0) * clamp(r1v * 8.0,  0.0, 1.0);

        float r2u = fract(vTexCoord.x * 25.0 + vTexCoord.y * 3.0 + uTime * 0.07 + 0.3);
        float r2v = fract(vTexCoord.y * 6.0  - uTime * 9.0  + 0.5);
        float s2  = step(0.97,  r2u) * clamp(1.0 - r2v * 2.5, 0.0, 1.0) * clamp(r2v * 6.0,  0.0, 1.0);

        float r3u = fract(vTexCoord.x * 15.0 + vTexCoord.y * 1.5 + uTime * 0.05 + 0.7);
        float r3v = fract(vTexCoord.y * 12.0 - uTime * 15.0 + 0.2);
        float s3  = step(0.985, r3u) * clamp(1.0 - r3v * 1.5, 0.0, 1.0) * clamp(r3v * 10.0, 0.0, 1.0);

        float rain = clamp(s1 * 0.5 + s2 * 0.4 + s3 * 0.3, 0.0, 0.8);
        finalRGB = mix(finalRGB, vec3(0.75, 0.85, 1.0), rain);

        // Lightning -- three interfering sine waves raised to power 3 for sharp irregular spikes
        float lt1      = sin(uTime * 1.3) * sin(uTime * 7.7)  * sin(uTime * 13.1);
        float lt2      = sin(uTime * 2.1) * sin(uTime * 5.3)  * sin(uTime * 17.9);
        float flash    = pow(max(0.0, lt1 * lt2 * 4.0), 3.0) * 2.5;
        finalRGB       = finalRGB + vec3(0.85, 0.90, 1.0) * flash;

        // Blue-grey storm tint over the whole scene
        finalRGB = mix(finalRGB, vec3(0.04, 0.06, 0.10), 0.28);
    }

    vOutputColour = vec4(finalRGB, litColour.a);
}
