#version 330 core

in vec2 Texcoord;
in vec2 TexcoordStart;
in float DistanceFromCamera;

out vec4 fragColor;

uniform sampler2D tex;

void main() {
   float blockTexSize = 16.0 / 128.0;
   vec2 unTiledCoord = Texcoord % blockTexSize;

   float maxDistance = 24.0; // full fade will be at this distance
   float fadeFactor = clamp(1.0 - (DistanceFromCamera / maxDistance), 0.0, 1.0);

   vec4 texColor = texture(tex, TexcoordStart + unTiledCoord);
   fragColor = vec4(texColor.rgb * fadeFactor, texColor.a);
}