varying vec3 vVertex;
varying vec3 vNormal;

const vec3 lightPos = vec3(10.0, 10.0, 10.0);
const vec4 ambient = vec4(0.0, 0.0, 0.0, 0.0);
const vec4 diffuse = vec4(0.2, 0.2, 0.2, 1.0);
const vec4 specular = vec4(0.2, 0.2, 0.2, 1.0);

const vec4 material = vec4(1.0, 1.0, 1.0, 1.0);
const float shininess = 6.0;

void main()
{
   vec3 L = normalize(lightPos - vVertex);
   vec3 E = normalize(-vVertex); // we are in Eye Coordinates, so EyePos is (0,0,0)
   vec3 R = normalize(-reflect(L, vNormal));

   //calculate Ambient Term:
   vec4 Iamb = material * ambient;

   //calculate Diffuse Term:
   vec4 Idiff = material * diffuse * max(dot(vNormal, L), 0.0);
   Idiff = clamp(Idiff, 0.0, 1.0);

   // calculate Specular Term:
   vec4 Ispec = material * specular * pow(max(dot(R, E), 0.0), 0.3 * shininess);
   Ispec = clamp(Ispec, 0.0, 1.0);

   // write Total Color:
   gl_FragColor = Iamb + Idiff + Ispec;
   gl_FragColor = gl_Color;
}
