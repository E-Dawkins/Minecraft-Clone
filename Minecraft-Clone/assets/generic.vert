#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in int faceIndex;

out vec2 Texcoord;
flat out int FaceIndex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
   Texcoord = texcoord;
   FaceIndex = faceIndex;
   gl_Position = proj * view * model * vec4(position, 1.0f);
}