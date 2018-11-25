uniform mat4 uViewProjectionMatrix;
uniform mat4 uViewMatrix;

layout(location = 0) in vec3 aVertex;
layout(location = 1) in vec3 aNormal;

out vec3 vVertex;
out vec3 vNormal;

void main()
{
	vVertex = aVertex;
	vNormal = aNormal;

    gl_Position = uViewProjectionMatrix * vec4(aVertex, 1.0);
}
