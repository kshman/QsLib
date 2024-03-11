uniform mat4 MVP;
uniform mat4 World;
uniform mat4 InvWorld;

attribute vec3 aPosition;
//attribute vec2 aCoord;
attribute vec3 aNormal;
attribute vec4 aColor;

varying vec3 vPosition;
varying vec3 vNormal;
varying vec4 vColor;
//varying vec2 vCoord;

void main()
{
    vPosition = (World * vec4(aPosition, 1.0)).xyz;
    vNormal = mat3(InvWorld) * aNormal;
    vColor = aColor;
    gl_Position = MVP * vec4(aPosition, 1.0);
}
