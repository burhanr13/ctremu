const char* vtxshadersrc = R"(
#version 460 core
layout (location=0) in vec4 pos;
layout (location=1) in vec4 color;
out vec4 outclr;

void main() {
    gl_Position = pos;
    outclr = color;
}
)";

const char* fragshadersrc = R"(
#version 460 core

in vec4 outclr;
out vec4 fragclr;

void main() {
    fragclr = outclr;
}
)";