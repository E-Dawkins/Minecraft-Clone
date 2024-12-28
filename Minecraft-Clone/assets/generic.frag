#version 330 core

in vec3 Color;
in vec2 Texcoord;

out vec4 fragColor;

uniform sampler2D tex;
uniform vec2 uvOffset;

void main() {
   vec2 remappedUV = (Texcoord + uvOffset) * 0.125f; // scale offset uvs down to 16/128 pixels
   fragColor = texture(tex, remappedUV);
}