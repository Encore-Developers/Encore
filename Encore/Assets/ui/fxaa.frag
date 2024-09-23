#version 330
 
#define FXAA_REDUCE_MIN (1.0/128.0)
#define FXAA_REDUCE_MUL (1.0/8.0)
#define FXAA_SPAN_MAX 8.0
 
in vec2 fragTexCoord;
 
out vec4 fragColor;
 
uniform sampler2D texture0;
uniform vec2 resolution;
 
void main()
{
    fragColor = texture2D(texture0, fragTexCoord.xy);
/*
    vec2 inverse_resolution = vec2(1.0/resolution.x,1.0/resolution.y);

    vec4 rgbNW = texture2D(texture0, fragTexCoord.xy + (vec2(-1.0,-1.0)) * inverse_resolution);
    vec4 rgbNE = texture2D(texture0, fragTexCoord.xy + (vec2(1.0,-1.0)) * inverse_resolution);
    vec4 rgbSW = texture2D(texture0, fragTexCoord.xy + (vec2(-1.0,1.0)) * inverse_resolution);
    vec4 rgbSE = texture2D(texture0, fragTexCoord.xy + (vec2(1.0,1.0)) * inverse_resolution);

    vec4 rgbM  = texture2D(texture0,  fragTexCoord.xy);

    vec4 luma = vec4(0.299, 0.587, 0.114, 0.2);

    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),dir * rcpDirMin)) * inverse_resolution;

    vec4 rgbA = 0.5 * (texture2D(texture0,   fragTexCoord.xy   + dir * (1.0/3.0 - 0.5)) + texture2D(texture0,   fragTexCoord.xy   + dir * (2.0/3.0 - 0.5)));
    vec4 rgbB = rgbA * 0.5 + 0.25 * (texture2D(texture0,  fragTexCoord.xy   + dir *  - 0.5) + texture2D(texture0,  fragTexCoord.xy   + dir * 0.5));

    float lumaB = dot(rgbB, luma);

    if((lumaB < lumaMin) || (lumaB > lumaMax))
    {
        fragColor = rgbA;
    }
    else
    {
        fragColor = rgbB;
    }
*/
}