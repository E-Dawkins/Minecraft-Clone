#version 330 core

in vec3 Color;
in vec2 Texcoord;

out vec4 fragColor;

uniform sampler2D tex;

void main() {
   fragColor = texture(tex, Texcoord);
}