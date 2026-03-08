#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;
in vec3 objPos;

uniform sampler2D maskTexture;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec4 noteColor;
uniform vec4 frameColor;
uniform vec3 specularLightPos;
uniform float time;
uniform float fadeStart;
uniform float fadeEnd;
uniform float trackLength = 20;
uniform float fadeSize = 3;

out vec4 finalColor;

float map(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

float mapclamp(float value, float min1, float max1, float min2, float max2) {
  return clamp(map(value, min1, max1, min2, max2), min2, max2);
}

void main()
{
    // vec2 push = vec2(fragTexCoord.x, fragTexCoord.y-time);
	float maskColor = texture(maskTexture, fragTexCoord).r;
    vec4 baseColor;
    baseColor = texture(texture0, fragTexCoord) * mix(frameColor, noteColor, maskColor);

    vec3 virtualLight = vec3(fragPosition.x, specularLightPos.y, specularLightPos.z);
    vec3 dirToLight = normalize(virtualLight - fragPosition);

    float shine = clamp(dot(fragNormal, dirToLight), 0, 1);
    shine = mapclamp(shine, 0.93, 1, 0, 1);
    shine = smoothstep(0, 1, shine);
    baseColor = mix(baseColor, vec4(1, 1, 1, 1), shine * maskColor * 0.3);
    baseColor.xyz += vec3(shine, shine, shine) * 0.3 * maskColor;
    //baseColor = vec4(shine, shine, shine, 1.0);

    //baseColor = vec4(fragNormal, 1);

    baseColor.a *= smoothstep(trackLength, trackLength-fadeSize, fragPosition.z);
    finalColor = baseColor;
}