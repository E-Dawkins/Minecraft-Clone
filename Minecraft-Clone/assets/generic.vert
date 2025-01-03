#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 offset;
layout (location = 3) in int faceId;
layout (location = 4) in vec2 texcoordStart;
layout (location = 5) in vec3 faceSize;

out vec2 Texcoord;
out vec2 TexcoordStart;

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

vec2 get2DFaceSize() {
   // since we know there is one axis set to '0'
   // we can easily ignore the 'empty' axis
   vec2 faceSize2D = vec2(0, 0);

   for (int i = 0; i < 3; i++) {
      if (faceSize[i] != 0) {
         if (faceSize2D.x == 0) {
            faceSize2D.x = faceSize[i];
         } else {
            faceSize2D.y = faceSize[i];
         }
      }
   }

   return faceSize2D;
}

void main() {
   float blockTexSize = 16.f / 128.f;
   Texcoord = (texcoord * blockTexSize) * get2DFaceSize();
   TexcoordStart = texcoordStart;

   vec3 offsetPos = (getRotatedPos() * faceSize) + offset;
   gl_Position = proj * view * vec4(offsetPos, 1.0);
}