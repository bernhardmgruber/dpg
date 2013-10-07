uniform mat4 uModelViewProjectionMatrix;
uniform mat4 uModelViewMatrix;

layout(location = 0) in vec3 aVertex;
layout(location = 1) in vec3 aNormal;

out vec3 vVertex;
out vec3 vNormal;

void main()
{
	vVertex = aVertex;
	vNormal = aNormal;

	gl_Position = uModelViewProjectionMatrix * vec4(aVertex, 1.0);
	//gl_FrontColor = gl_Color;
}
