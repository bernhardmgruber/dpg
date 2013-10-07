uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelViewMatrix;

//attribute vec3 aVertex;
//attribute vec3 aNormal;

varying vec3 vVertex;
varying vec3 vNormal;

void main()
{
	vVertex = vec3(gl_Vertex);
	vNormal = gl_Normal;

	gl_Position = uModelViewProjectionMatrix * gl_Vertex;
	//gl_FrontColor = gl_Color;
}
