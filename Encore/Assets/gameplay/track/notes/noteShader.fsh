#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;

uniform sampler2D maskTexture;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec4 noteColor;
uniform vec4 frameColor;
uniform float time;
uniform float fadeStart;
uniform float fadeEnd;
uniform float trackLength = 20;
uniform float fadeSize = 3;

out vec4 finalColor;

void main()
{
    // vec2 push = vec2(fragTexCoord.x, fragTexCoord.y-time);
	float maskColor = texture(maskTexture, fragTexCoord).r;
    vec4 baseColor;
    if (maskColor > 0.5) {
        baseColor = texture(texture0, fragTexCoord) * noteColor;
    } else {
        baseColor = texture(texture0, fragTexCoord) * frameColor;
    }
    baseColor = texture(texture0, fragTexCoord) * mix(frameColor, noteColor, maskColor);

    baseColor.a *= smoothstep(trackLength, trackLength-fadeSize, fragPosition.z);
    finalColor = baseColor;
}
