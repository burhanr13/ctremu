const char* mainvertsource = R"(
#version 410 core

layout (location=0) in vec2 pos;
layout (location=1) in vec2 inTexcoord;
out vec2 texcoord;

void main() {
    gl_Position = vec4(pos, 0, 1);
    texcoord = inTexcoord;
}

)";