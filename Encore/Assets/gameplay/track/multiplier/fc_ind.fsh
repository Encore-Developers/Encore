#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec4 BaseColor;
uniform float time;
uniform vec4 FCColor;
uniform float isFC;

// Output fragment color
out vec4 finalColor;

void main() {
    vec2 basePush = vec2(fragTexCoord.x+(time/8), fragTexCoord.y);
    vec2 middlePush = vec2(fragTexCoord.x-(time/5), fragTexCoord.y);
    vec2 topPush = vec2(fragTexCoord.x+(time/6), fragTexCoord.y);

    if (isFC > 0) {
        float proc = ((
            (texture(texture2, middlePush)) * 2
        ) +
        (
            (texture(texture1, topPush)) * 2
        ) +
        (
            (texture(texture0, basePush)) / 3
        )).r;
        float midPoint = 0.5;
        proc = clamp(proc, 0.0, 1.0)/1.2;
        float firstStep = clamp(smoothstep(0.0, midPoint, proc), 0, 1);
        float secondStep = clamp(smoothstep(midPoint, 1.0, proc), 0, 1);
        finalColor = mix(FCColor*0.7, FCColor, firstStep);
        finalColor = mix(finalColor, mix(FCColor, vec4(1.0, 1.0, 1.0, 1.0), 0.93), secondStep);
		//finalColor = (texture(texture2, middlePush) * 2) + (texture(texture1, topPush) * 2) + (texture(texture0, basePush) / 4);
        finalColor.a = 1.0f;
    }
    else {
        finalColor = BaseColor;
    }
}