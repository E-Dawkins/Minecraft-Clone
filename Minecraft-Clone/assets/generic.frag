#version 330 core

in vec2 Texcoord;
in vec2 TexcoordStart;

out vec4 fragColor;

uniform sampler2D tex;

void main() {
   float blockTexSize = 16.0 / 128.0;
   vec2 unTiledCoord = Texcoord % blockTexSize;

   fragColor = texture(tex, TexcoordStart + unTiledCoord);
}