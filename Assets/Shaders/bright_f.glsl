#version 400

uniform sampler2D u_tex;

// Fragment Shader Inputs
in VertexData
{
	vec3 normal;
	vec3 texCoord;
	vec4 colour;
	vec3 posEye;
} vIn;

layout(location = 0) out vec4 FragColor;

void main()
{
	vec2 uv = (vIn.texCoord).xy;

	vec3 rgb = texture(u_tex, uv).rgb;

	float BloomThreshold = 0.2;
	FragColor.xyz = clamp( (rgb - BloomThreshold) / (1.0 - BloomThreshold), vec3(0.0), vec3(1.0));
	FragColor.w = 1.0;
}