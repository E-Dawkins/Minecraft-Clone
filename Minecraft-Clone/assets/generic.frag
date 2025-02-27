#version 330 core

in vec2 Texcoord;
in vec2 TexcoordStart;
in float DistanceFromCamera;

out vec4 fragColor;

uniform sampler2D tex;
uniform uint renderDist;

void main() {
   float blockTexSize = 16.0 / 128.0;
   vec2 unTiledCoord = Texcoord % blockTexSize;

   float maxDistance = max((renderDist * 16.0) - 8.0, 8.0); // full fade will be at this distance
   float fadeFactor = clamp(DistanceFromCamera / maxDistance, 0.0, 1.0);

   vec4 texColor = texture(tex, TexcoordStart + unTiledCoord);
   vec3 fadeColor = vec3(0, 0, 0);
   fragColor = vec4(mix(texColor.rgb, fadeColor, fadeFactor), texColor.a);
}