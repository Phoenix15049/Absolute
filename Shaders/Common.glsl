layout(std140) uniform FrameData
{
	mat4 view;
	mat4 projection;
	mat4 viewProjection;
	mat4 inverseViewProjection;
	mat4 lightSpace;
	vec4 cameraPosition;
	vec4 skyTop;
	vec4 skyHorizon;
	vec4 groundColor;
	vec4 shadowParams;
	ivec4 options;
};
