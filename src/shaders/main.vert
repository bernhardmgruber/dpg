uniform mat4 uViewProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uNormalMatrix;

layout(location = 0) in vec3 aVertex;
layout(location = 1) in vec3 aNormal;

out vec3 vVertex;
out vec3 vNormal;
out vec3 vNormalUntransformed;

void main() {
	vec4 v = uViewMatrix * vec4(aVertex, 1.0);
	vVertex = vec3(v) / v.w;
	vNormalUntransformed = aNormal;
	vNormal = vec3(uNormalMatrix * vec4(aNormal, 1.0));

	gl_Position = uViewProjectionMatrix * vec4(aVertex, 1.0);
}
