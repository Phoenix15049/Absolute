#version 330 core

#include "Common.glsl"

uniform float gridExtent;

out vec3 vWorldPosition;

void main()
{
	const vec2 corners[6] = vec2[](vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(1.0, 1.0),
	                               vec2(-1.0, -1.0), vec2(1.0, 1.0), vec2(-1.0, 1.0));

	vec2 center = floor(cameraPosition.xz / 10.0) * 10.0;
	vec2 position = center + corners[gl_VertexID] * gridExtent;

	vWorldPosition = vec3(position.x, 0.0, position.y);
	gl_Position = viewProjection * vec4(vWorldPosition, 1.0);
}
