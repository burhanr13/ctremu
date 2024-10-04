const char* mainfragsource = R"(
#version 410 core

in vec2 texcoord;
out vec4 fragclr;

uniform sampler2D tex;

void main() {
    fragclr = texture(tex, texcoord);
}

)";
