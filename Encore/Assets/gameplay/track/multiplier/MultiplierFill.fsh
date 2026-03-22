#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform vec4 BaseColor;
uniform vec4 MultiplierColor;
uniform float FillPercentage;

out vec4 FragColor;

void main()
{
    // vec4 EmptyColor = vec4(0.3,0.0,0.3,1.0);
    vec4 fillColor = MultiplierColor;
    if (fragTexCoord.x < FillPercentage) {
        FragColor = vec4(fillColor.x, fillColor.y, fillColor.z, 1.0);
    } else {
        FragColor = vec4(BaseColor.x, BaseColor.y, BaseColor.z, 1.0);
    }
}