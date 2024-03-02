uniform mat4 WorldViewProj;

attribute vec3 aPosition;
//attribute vec2 aCoord;
//attribute vec3 aNormal;
attribute vec4 aColor;

//varying vec2 vCoord;
varying vec4 vColor;

void main()
{
    // Send vertex attributes to fragment shader
    //vCoord = aCoord;
    vColor = aColor;

    // Calculate final vertex position
    gl_Position = WorldViewProj * vec4(aPosition, 1.0);
}
