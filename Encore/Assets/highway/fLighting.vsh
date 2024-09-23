#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// NOTE: Add here your custom variables

void main()
{
    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));
    vec3 finalPos = vertexPosition;
    finalPos.y += (-(fragPosition.x * fragPosition.x))/(100+fragPosition.y);
    if (vertexPosition.x > 0)
        {
            finalPos.x -= (vertexPosition.y * vertexPosition.y)/(75);
        }
        else
        {
            finalPos.x += (vertexPosition.y * vertexPosition.y)/(75);
        }
    gl_Position = mvp*vec4(finalPos, 1.0);
}