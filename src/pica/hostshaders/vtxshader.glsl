const char* vtxshadersrc = R"(
#version 410 core

layout (location=0) in vec4 pos;
layout (location=1) in vec4 color;
out vec4 outclr;

void main() {
    gl_Position.x = -pos.y;
    gl_Position.y = pos.x;
    gl_Position.z = pos.z;
    gl_Position.w = pos.w;
    outclr = color;
}

)";