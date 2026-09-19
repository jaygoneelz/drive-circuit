#version 400 core

// diamond.frag -- INM376 Final Coursework 2026
// Applies a texture multiplied by the interpolated Phong colour from diamond.vert.

in vec3 vColour;
in vec2 vTexCoord;

out vec4 vOutputColour;

uniform sampler2D sampler0;
uniform bool      bUseTexture;

void main()
{
    if (bUseTexture)
        vOutputColour = texture(sampler0, vTexCoord) * vec4(vColour, 1.0);
    else
        vOutputColour = vec4(vColour, 1.0);
}
