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
    vec4 glow = vec4(color.xyz, mix(GlowIntensity, 0, smoothstep(SolidPct, GlowBottomPct, pos.y)));
    return mix(color, glow, smoothstep(SolidPct, SolidPct+0.02, pos.y));
}

vec4 sample(vec2 pos) {
    vec4 main = meter(pos, MainColor);
    vec4 resid = meter(pos, ResidualColor);
    float epsilon = 0;
    if (FillResidualPct > 0.01) {
        epsilon = 0.01;
    }
    vec4 filled = mix(main, resid, smoothstep(FillPct, FillPct+epsilon, pos.x));
    return mix(filled, vec4(0.0), smoothstep(FillResidualPct, FillResidualPct+epsilon, pos.x));
}

void main() {
    vec4 accumulate = vec4(0.0);
    accumulate += sample(fragTexCoord+vec2(0.001, 0.0));
    accumulate += sample(fragTexCoord+vec2(-0.001, 0.0));
    accumulate += sample(fragTexCoord+vec2(0, 0.001));
    accumulate += sample(fragTexCoord+vec2(0, -0.001));
    accumulate += sample(fragTexCoord);
    accumulate /= 5;
    finalColor = accumulate * vec4(1.0, 1.0, 1.0, Fade);
}