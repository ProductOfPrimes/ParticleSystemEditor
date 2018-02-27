#version 400

uniform vec4 u_lightPos;
uniform vec4 u_colour;

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
	vec3 N = normalize(vIn.normal); // Normal from VBO (obj file)

	float diffuse = max(0.0, dot(N, L));
	
	// Write to color fbo target
	FragColor = vec4(vec3(0.5, 0.5, 0.5) * (diffuse * 0.8f) + u_colour.rgb, 1.0);
	
	// Write to normal fbo target
	FragNormal = vec4(N * 0.5 + 0.5, 1.0);
}