#version 330 core

in vec2 Texcoord;
in float DistanceFromCamera;
flat in uint TextureId;

out vec4 fragColor;

uniform sampler2D tex;
uniform uint renderDist;

void main() {
   float maxDistance = max((renderDist * 16.0) - 8.0, 8.0); // full fade will be at this distance
   float fadeFactor = clamp(DistanceFromCamera / maxDistance, 0.0, 1.0);

   vec2 uvOffset = vec2(TextureId % 4u, TextureId / 4u) * 0.25f;
   vec4 texColor = texture(tex, Texcoord + uvOffset);
   vec3 fadeColor = vec3(0, 0, 0);
   fragColor = vec4(mix(texColor.rgb, fadeColor, fadeFactor), texColor.a);
}