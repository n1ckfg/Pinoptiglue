#version 310 es

precision mediump float;

uniform sampler2D tex0;
uniform sampler2D tex1;

in vec2 varyingtexcoord;
out vec4 outputColor;

const float gamma = 1.2;
const vec2 texelSize = vec2(8.0, 8.0); //1.0/120.0, 1.0/90.0);
const float posterizeLevels = 16.0; //90.0;

const float kernel[5] = float[](0.10, 0.20, 0.40, 0.20, 0.10);

float getLuminance(vec3 col) {
    return dot(col, vec3(0.299, 0.587, 0.114));
}

float map(float s, float a1, float a2, float b1, float b2) {
    return b1 + (s - a1) * (b2 - b1) / (a2 - a1);
}

vec3 adjustGamma(vec3 color, float gamma) {
    return pow(color, vec3(1.0 / gamma));
}

void main() {
    vec2 uv = vec2(varyingtexcoord.x, 1.0 - varyingtexcoord.y);
    uv.y = abs(uv.y - 1.0);

    vec3 centerColor = texture2D(tex0, uv).xyz + texture2D(tex1, uv).xyz;   
    vec3 leftColor = texture2D(tex1, uv - vec2(texelSize.x, 0.0)).xyz;
    vec3 rightColor = texture2D(tex0, uv + vec2(texelSize.x, 0.0)).xyz;
    vec3 topColor = texture2D(tex1, uv + vec2(0.0, texelSize.y)).xyz;
    vec3 bottomColor = texture2D(tex0, uv - vec2(0.0, texelSize.y)).xyz;

    vec3 blurredColor = topColor * kernel[0] + leftColor * kernel[1] + centerColor * kernel[2] + rightColor * kernel[3] + bottomColor * kernel[4];
    vec3 sharpenedColor = blurredColor * 5.0 - (leftColor + rightColor + topColor + bottomColor);
    vec3 posterizedColor = floor(sharpenedColor * posterizeLevels) / posterizeLevels;

    outputColor = vec4(posterizedColor, 1.0);
}
