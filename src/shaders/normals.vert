#version 330 core

uniform mat4 uViewProjectionMatrix;

layout(location = 0) in vec3 aVertex;
layout(location = 1) in vec3 aNormal;

out vec3 vVertex;
out vec3 vNormal;

void main()
{
    vVertex = aVertex;
    vNormal = aNormal;
}
