#version 400

uniform vec4 u_lightPos;
uniform vec4 u_colour;
uniform mat4 u_mv;
uniform sampler2D u_normalMap;

// Fragment Shader Inputs
in VertexData
{
	vec3 normal;
	vec3 texCoord;
	vec4 colour;
	vec3 posEye;
} vIn;

// Multiple render targets!
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragNormal;

void main()
{
	vec3 L = normalize(u_lightPos.xyz - vIn.posEye);
	//vec3 N = normalize(vIn.normal); // Normal from VBO (obj file)

	vec3 N = texture(u_normalMap, vIn.texCoord.xy).xyz;
	N = N * 2.0 - 1.0; // deserialize / unpack normal 
	N = (u_mv * vec4(N, 0.0)).xyz; // transformed / rotated normal
	N = normalize(N); // ensure normal is of length 1

	

	// Calculate specular term
	vec3 specColor = vec3(1.0);
	float specIntensity = 0.25;
	float specPower = 32.0f; // roughness
	vec3 V = -vIn.posEye; // view vector
	V = normalize(V); // get the direction of the camera to the surface
	vec3 H = normalize (V + L);
	float ndoth = max(0.0, dot(N, H));
	vec3 specular = pow(ndoth, specPower) * specIntensity * specColor;

	// calculate diffuse term
	float ndotl = max(0.0, dot(N, L));
	float diffuseIntensity = 1.0;
	vec3 diffuseColor = u_colour.xyz;
	vec3 diffuse = ndotl * diffuseIntensity * diffuseColor;


	// Write to color fbo target
	//FragColor = vec4(vec3(0.5, 0.5, 0.5) * (ndotl * 0.8f) + u_colour.rgb, 1.0);

	//FragColor = vec4(specular, 1.0); // specular highlights only

	FragColor = vec4(specular + diffuse, 1.0);



	// Write to normal fbo target
	FragNormal = vec4(N * 0.5 + 0.5, 1.0);
}
