#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float time;
uniform float trackLength = 20;
uniform float fadeSize = 3;

out vec4 finalColor;

void main()
{
    vec2 push = vec2(fragTexCoord.x, fragTexCoord.y-time);

    vec4 baseColor = texture(texture0, push) * fragColor * colDiffuse;
    float strikelineGlow = clamp(smoothstep(0, trackLength*0.8, fragPosition.z), 0, 1);
    if (fragPosition.z < 0) {
        strikelineGlow = 0;
    }
    float mult = mix(1, 0.55, strikelineGlow);
    baseColor.a *= smoothstep(trackLength, trackLength-fadeSize, fragPosition.z);
    baseColor.r *= pow(mult, 1.5);
    baseColor.g *= pow(mult, 1.5);
    baseColor.b *= pow(mult, 1.5);
    baseColor.a *= mult;
    finalColor = baseColor;
}