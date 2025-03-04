#version 330 core

in vec2 Texcoord;
in float DistanceFromCamera;
flat in uint TextureId;

out vec4 fragColor;

uniform sampler2D tex;

void main() {
   float maxDistance = 24.0; // full fade will be at this distance
   float fadeFactor = clamp(1.0 - (DistanceFromCamera / maxDistance), 0.0, 1.0);

   vec2 uvOffset = vec2(TextureId % 4u, TextureId / 4u) * 0.25f;
   vec4 texColor = texture(tex, Texcoord + uvOffset);
   fragColor = vec4(texColor.rgb, texColor.a);
}