#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec2 uvOffset;
// Output fragment color
out vec4 finalColor;


void main()
{
    // Texel color fetching from texture sampler
    vec4 baseColor = texture(texture0, fragTexCoord+vec2(uvOffset.x,uvOffset.y));
    finalColor = baseColor;
}