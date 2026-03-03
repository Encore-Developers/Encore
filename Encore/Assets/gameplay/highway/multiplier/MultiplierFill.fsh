#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D BaseTexture;
uniform vec4 MultiplierColor;
uniform float FillPercentage;

out vec4 FragColor;

void main()
{
    vec4 EmptyColor = vec4(0.3,0.0,0.3,1.0);
    vec4 TextureColor = texture(BaseTexture, fragTexCoord);
    vec4 InvertedTexture = vec4(1.0-TextureColor.x, 1.0-TextureColor.y, 1.0-TextureColor.z, TextureColor.w);
    vec4 baseColor = mix(EmptyColor, clamp(InvertedTexture, 0.0, 0.5), 0.2);
    vec4 fillColor = MultiplierColor;
    if (fragTexCoord.x < FillPercentage) {
        FragColor = vec4(fillColor.x, fillColor.y, fillColor.z, 1.0);
    } else {
        FragColor = vec4(baseColor.x, baseColor.y, baseColor.z, 1.0);
    }
}