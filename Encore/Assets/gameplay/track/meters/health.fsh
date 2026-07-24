#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform vec4 MainColor;
uniform vec4 ResidualColor;
uniform sampler2D FillTexture;
uniform sampler2D texture0;
uniform float FillPct;
uniform float FillResidualPct;
uniform float SolidPct;
uniform float GlowBottomPct;
uniform float GlowIntensity;
uniform float Fade;
out vec4 finalColor;

vec4 meter(vec2 pos, vec4 color) {
    if (pos.y < SolidPct) {
        return color;
    } else {
        return vec4(color.xyz, mix(GlowIntensity, 0, smoothstep(SolidPct, GlowBottomPct, pos.y)));
    }
}

vec4 sample(vec2 pos) {
    if (pos.x <= FillPct) {
        return meter(pos, MainColor);
    } else if (pos.x <= FillResidualPct) {
        return meter(pos, ResidualColor);
    }
    return vec4(0.0);
}

void main() {
    finalColor = sample(fragTexCoord) * vec4(1.0, 1.0, 1.0, Fade);
}