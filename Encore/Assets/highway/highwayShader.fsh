#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;

uniform sampler2D highwayTex;
uniform vec4 highwayColor;
uniform float time;
uniform float fadeStart;
uniform float fadeEnd;

out vec4 finalColor;

void main()
{
    vec2 push = vec2(fragTexCoord.x, fragTexCoord.y-time);
    vec4 baseColor = texture2D(highwayTex, push) * fragColor * highwayColor;
    baseColor.a *= smoothstep(fadeEnd, fadeStart, fragPosition.z);
    finalColor = baseColor;
}