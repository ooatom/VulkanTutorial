#version 450

layout (location=0) in vec4 fragColor;

layout (location=0) out vec4 outColor;

void main() {
    vec2 coord = gl_PointCoord - 0.5;
    float alpha = (0.5 - length(coord)) * 1.5;
    outColor = vec4(fragColor.xyz, alpha);
}