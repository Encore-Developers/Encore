#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matNormal;
uniform float trackLength = 20;
uniform float fadeSize = 3;
uniform float curveFac = 50;
uniform float offset = 0;
uniform float scale = 1;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out vec3 objPos;

// NOTE: Add here your custom variables

void main()
{
    mat4 ModelViewMatrix = matView * matModel;
    mat4 ViewMatrixInverse = inverse(matView);

    vec4 PositionInViewSpace = ModelViewMatrix * vec4(vertexPosition, 1.0);
    vec4 PositionInWorldSpace = ViewMatrixInverse * PositionInViewSpace;
    float yPos = PositionInWorldSpace.y;
    float xPos = PositionInWorldSpace.x;

    float xPow = pow(xPos, 2);
    float yPow = pow(yPos, 2);
    // Send vertex attributes to fragment shader
    fragPosition = vec3((matModel) * vec4(vertexPosition, 1.0));;
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));
    PositionInWorldSpace.y = PositionInWorldSpace.y + (-xPow) / (curveFac);
    // PositionInWorldSpace.x = PositionInWorldSpace.x + (-(yPow)) / (curveFac * 2);
    PositionInViewSpace = matView * PositionInWorldSpace;
    vec4 PositionInObjectSpace = inverse(ModelViewMatrix) * PositionInViewSpace;
    gl_Position = mvp * PositionInObjectSpace;
    gl_Position /= vec4(gl_Position.w, gl_Position.w, gl_Position.w, 1);
    //fragNormal = (ModelViewMatrix * vec4(fragNormal, 0.0)).xyz;

    gl_Position -= vec4(0, -1, 0, 0);
    gl_Position *= vec4(scale, scale, 1, 1);
    gl_Position += vec4(0, -1, 0, 0);

    gl_Position += vec4(offset, 0, 0, 0);
    gl_Position *= vec4(gl_Position.w, gl_Position.w, gl_Position.w, 1);
    //float fade = (fadeSize-clamp(vertexPosition.z - (trackLength-fadeSize), 0, fadeSize))/fadeSize;
    //fragColor.a *= fade;
}