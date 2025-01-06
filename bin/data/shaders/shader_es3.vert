uniform mat4 modelViewProjectionMatrix;
uniform sampler2D tex0;

attribute vec4 position;
attribute vec2 texcoord;

varying vec2 texCoordVarying;

void main()
{
    texCoordVarying = texcoord;

    gl_Position = modelViewProjectionMatrix * position;
}