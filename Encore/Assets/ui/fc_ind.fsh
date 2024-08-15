#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D baseTex;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform float time;
uniform vec4 color;
uniform int isFC;

// Output fragment color
out vec4 finalColor;

void main() {
    vec2 basePush = vec2(fragTexCoord.x+(time/8), fragTexCoord.y);
    vec2 middlePush = vec2(fragTexCoord.x-(time/5), fragTexCoord.y);
    vec2 topPush = vec2(fragTexCoord.x+(time/6), fragTexCoord.y);

    vec4 baseColor = color;

    finalColor = clamp((((baseColor * texture2D(tex2, middlePush))*2) + ((baseColor * texture2D(tex1, topPush))*2) + ((baseColor * texture2D(baseTex, basePush))/4))*5.0f, baseColor*1.5f, vec4(1.0f,1.0f,1.0f,1.0f));
    finalColor.a = 1.0f;
}