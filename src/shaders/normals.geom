#version 330 core

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

uniform mat4 uViewProjectionMatrix;

uniform float normalLength = 0.05;

in vec3 vVertex[];
in vec3 vNormal[];

void main()
{
    for(int i = 0; i < gl_in.length (); i++)
    {
        gl_Position = uViewProjectionMatrix * vec4(vVertex[i], 1.0f);
        EmitVertex();

        gl_Position = uViewProjectionMatrix * vec4(vVertex[i] + vNormal[i] * normalLength, 1.0f);
        EmitVertex();

        EndPrimitive();
    }
}
