#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;

uniform sampler2D texture0;
uniform float fadeStart;
uniform float fadeEnd;
uniform vec4 colorForAccent;
uniform int useInAccent;
// Output fragment color
out vec4 finalColor;


void main()
{
    vec4 ColorAccent = colorForAccent;
    if (useInAccent == 1) {
        ColorAccent = fragColor;
    }
    vec4 baseColor = texture(texture0, fragTexCoord) * ColorAccent;
    baseColor.a *= smoothstep(fadeEnd, fadeStart, fragPosition.z);
    finalColor = baseColor; // baseColor
}