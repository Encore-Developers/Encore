#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform vec4 FillColor;
uniform sampler2D FillTexture;
uniform sampler2D texture0;
uniform lowp float FillPct;

out vec4 finalColor;

void main() {
    vec4 textureColor = texture(texture0, fragTexCoord);

    // should this spot be colored if overdrive fills here?
    float maskOn = texture(FillTexture, fragTexCoord).a;

    // should this spot be filled because of overdrive?
    float overdriveFill=step(FillPct,1.0-fragTexCoord.x);

    finalColor = mix(textureColor, FillColor, maskOn*overdriveFill);
    // if (maskOn > 0.0) {
    //    if (overdriveFill > 0.0) {
    //        finalColor = FillColor;
    //    } else {
    //        finalColor = textureColor;
    //    }
    //} else {
    //    finalColor = textureColor;
    //}
}