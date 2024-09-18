const char* fragshadersrc = R"(
#version 410 core

in vec4 outclr;
out vec4 fragclr;

void main() {
    fragclr = outclr;
}

)";
