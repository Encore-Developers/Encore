#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;

uniform sampler2D texture0;
uniform float fadeStart;
uniform float fadeEnd;
// Output fragment color
out vec4 finalColor;


void main()
{
    // Texel color fetching from texture sampler
    vec4 baseColor = texture(texture0, fragTexCoord);
    baseColor.a *= smoothstep(fadeEnd, fadeStart, fragPosition.z);
    finalColor = baseColor;
}