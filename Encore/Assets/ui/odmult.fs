#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D fillTex;
uniform float overdrive;
uniform float comboCounter;
uniform float multBar;
uniform int isBassOrVocal;
// Output fragment color
out vec4 finalColor;


void main()
{
    // Texel color fetching from texture sampler
    vec4 baseColor = texture(texture0, fragTexCoord);
    vec4 fillColor = texture(fillTex, fragTexCoord);
    vec2 fragPos = fragTexCoord;
    float overdriveY=step(0.95,1.0-fragPos.y);
    float fill3Y=step(0.8,1.0-fragPos.y);
    float fill5Y=step(0.65,1.0-fragPos.y);
    float fillComboY=step(0.51,1.0-fragPos.y);
    fillComboY-=fill5Y;
    fill5Y-=fill3Y;
    fill3Y-=overdriveY;
    float overdriveFill=step(overdrive,1.0-fragPos.x);
    float multBarFill=step(multBar,fragPos.x);
    float comboCounterFill=step(comboCounter,fragPos.x);
    if(isBassOrVocal==1)
        finalColor = mix(mix(mix(baseColor,mix(fillColor,baseColor,overdriveFill),overdriveY),mix(fillColor,baseColor,multBarFill),fill5Y),mix(fillColor,baseColor,comboCounterFill),fillComboY);
    else
        finalColor = mix(mix(mix(baseColor,mix(fillColor,baseColor,overdriveFill),overdriveY),mix(fillColor,baseColor,multBarFill),fill3Y),mix(fillColor,baseColor,comboCounterFill),fillComboY);
}