#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 offset;
layout (location = 3) in int faceId;
layout (location = 4) in vec2 texcoordStart;

out vec2 Texcoord;
flat out int FaceId;

uniform mat4 view;
uniform mat4 proj;

vec3 getRotatedPos() {
   switch (faceId) {
      case 0: return vec3( position.x, -position.z, position.y);
      case 1: return vec3(-position.x,  position.z, position.y);
      case 2: return vec3(-position.z, -position.x, position.y);
      case 3: return vec3( position.z,  position.x, position.y);
      case 5: return vec3( position.x, -position.y, position.z);
      default: return position;
   }
}

void main() {
   float blockTexSize = 16.f / 128.f;
   Texcoord = texcoordStart + texcoord * blockTexSize;
   FaceId = faceId;

   vec3 offsetPos = getRotatedPos() + offset;
   gl_Position = proj * view * vec4(offsetPos, 1.0);
}