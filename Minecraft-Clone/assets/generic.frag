#version 330 core

in vec2 Texcoord;
flat in int FaceIndex;

out vec4 fragColor;

uniform sampler2D tex;
uniform vec2 uvOffsets[6];

void main() {
   vec2 remappedUV = (Texcoord + uvOffsets[FaceIndex]) * 0.125f; // scale offset uvs down to 16/128 pixels
   fragColor = texture(tex, remappedUV);
}