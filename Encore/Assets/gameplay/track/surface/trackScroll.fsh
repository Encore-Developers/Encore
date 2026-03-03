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
    baseColor.a *= smoothstep(trackLength, trackLength-fadeSize, fragPosition.z);
    finalColor = baseColor;
}