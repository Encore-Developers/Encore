#version 330

in vec2 fragTexCoord;
uniform sampler2D texture0;
uniform float time;
out vec4 finalColor;

vec2 rand(vec2 co) {
    return fract(sin(vec2(dot(co, vec2(12.9898, 78.233)), dot(co, vec2(12.9898, 78.233)))) * 43758.5453);
}

float when_lt(float x, float y) {
  return max(sign(y - x), 0.0);
}

float when_ge(float x, float y) {
  return 1.0 - when_lt(x, y);
}

float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    float a = dot(rand(i), f);
    float b = dot(rand(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0));
    float c = dot(rand(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0));
    float d = dot(rand(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0));
    vec2 u = smoothstep(0.0, 1.0, f);
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}


void main()
{
vec2 uv = fragTexCoord;
vec2 uvDist=uv;
uv.x*=3.0;
uv.y*=3.0;

uvDist.x+=sin(cos(uv.x)+(time)+0.15)*0.5*noise(uv);
uvDist.y+=sin(cos(uv.y)+(time))*0.5*noise(uv);
uvDist.x-=2.0*(when_ge(uvDist.x,1.0));
uvDist.x=abs(uvDist.x);
uvDist.y-=2.0*(when_ge(uvDist.y,1.0));
uvDist.y=abs(uvDist.y);
finalColor = texture(texture0, uvDist);
}