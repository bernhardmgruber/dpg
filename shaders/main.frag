in vec3 vVertex;
in vec3 vNormal;

//const vec3 lightPos = vec3(-10.0, 10.0, 10.0);
const vec3 invLightDir = vec3(0.57735, 0.57735, 0.57735);
const vec4 ambient = vec4(0.3, 0.3, 0.3, 1.0);
const vec4 diffuse = vec4(0.3, 0.5, 0.7, 1.0);
const vec4 specular = vec4(0.1, 0.1, 0.1, 1.0);

const vec4 materialAmbient = vec4(0.3, 0.3, 0.3, 1.0);
const vec4 materialDiffuse = vec4(0.3, 0.3, 0.3, 1.0);
const vec4 materialSpecular = vec4(0.3, 0.3, 0.3, 1.0);
const float shininess = 1.0;

void main()
{
	//vec3 L = normalize(lightPos - vVertex);
	vec3 L = invLightDir;
	vec3 E = normalize(-vVertex); // we are in Eye Coordinates, so EyePos is (0,0,0)
	vec3 R = normalize(-reflect(L, vNormal));

	// calculate ambient Term:
	vec4 Iamb = materialAmbient * ambient;

	// calculate diffuse Term:
	vec4 Idiff = materialDiffuse * diffuse * max(dot(vNormal, L), 0.0);
	Idiff = clamp(Idiff, 0.0, 1.0);

	// calculate specular Term:
	vec4 Ispec = materialSpecular * specular * pow(max(dot(R, E), 0.0), 0.3 * shininess);
	Ispec = clamp(Ispec, 0.0, 1.0);

	// write Total Color:
	gl_FragColor = Iamb + Idiff + Ispec;
}