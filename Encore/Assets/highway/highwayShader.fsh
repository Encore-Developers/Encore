#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float time;
uniform float fadeStart;
uniform float fadeEnd;

out vec4 finalColor;

void main()
{
    vec2 push = vec2(fragTexCoord.x, fragTexCoord.y-time);
    vec4 baseColor = texture2D(texture0, push) * fragColor * colDiffuse;
    baseColor.a *= smoothstep(fadeEnd, fadeStart, fragPosition.z);
    baseColor.r *= fragPosition.z < 2.4 ? 1 : 0.5f;
    baseColor.g *= fragPosition.z < 2.4 ? 1 : 0.5f;
    baseColor.b *= fragPosition.z < 2.4 ? 1 : 0.5f;
    finalColor = baseColor;
}