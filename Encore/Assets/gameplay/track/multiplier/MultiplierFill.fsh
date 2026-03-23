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
    float frac = 1 - FillPercentage;
    FragColor = mix(BaseColor, fillColor, smoothstep(frac-0.005, frac+0.005, fragTexCoord.x));
}