uniform mat4 uModelViewProjectionMatrix;

attribute vec3 aVertex;

void main()
{
	gl_Position = uModelViewProjectionMatrix * vec4(aVertex, 1.0);
}
