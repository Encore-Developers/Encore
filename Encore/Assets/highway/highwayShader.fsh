in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;

uniform sampler2D texture0;
uniform float curveMax;
// Output fragment color
out vec4 finalColor;

void main()
{
    float distance2 = abs(fragPosition.y);

    float scale = pow(distance2, 0.5) * 0.01;

    vec2 newFragPos = fragTexCoord;
    newFragPos.y -= scale;

    finalColor = texture(texture0, newFragPos);
}