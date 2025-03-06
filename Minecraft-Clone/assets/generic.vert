#version 330 core

// Vertex data
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;

// Face data
layout (location = 2) in uint blockPos;
layout (location = 3) in uint directionId;

out vec2 Texcoord;
out float DistanceFromCamera;
flat out uint TextureId;

uniform mat4 view;
uniform mat4 proj;
uniform vec2 chunkIndex;

vec3 getRotatedPos(uint direction) {
   switch (direction) {
      case 0: return vec3( position.x, -position.z, position.y);
      case 1: return vec3(-position.x,  position.z, position.y);
      case 2: return vec3(-position.z, -position.x, position.y);
      case 3: return vec3( position.z,  position.x, position.y);
      case 5: return vec3( position.x, -position.y, position.z);
      default: return position;
   }
}

vec3 getFaceOffset(uint direction) {
   switch (direction) {
      case 0: return vec3(0, -0.5f, 0);
      case 1: return vec3(0,  0.5f, 0);
      case 2: return vec3(-0.5f, 0, 0);
      case 3: return vec3( 0.5f, 0, 0);
      case 5: return vec3(0, 0, -0.5f);
      default: return vec3(0, 0, 0.5f);
   }
}

void main() {
   uint iDirection = (directionId >> 4) & 15u;
   vec3 vBlockPos = vec3((blockPos >> 12) & 15u, (blockPos >> 8) & 15u, blockPos & 255u);

   vec3 offsetPos = getRotatedPos(iDirection) + vBlockPos + getFaceOffset(iDirection) + vec3(chunkIndex * vec2(16.f, 16.f), 0);
   vec4 viewPos = view * vec4(offsetPos, 1.0);
   gl_Position = proj * viewPos;

   Texcoord = texcoord / 4.f; // remap from 0-1 to 0-0.25
   DistanceFromCamera = length(viewPos.xyz);
   TextureId = directionId & 15u;
}