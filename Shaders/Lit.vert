#version 330 core

#include "Common.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec4 inTangent;

uniform mat4 model;
uniform mat3 normalMatrix;

out vec3 vWorldPosition;
out vec3 vNormal;
out vec2 vUV;
out vec4 vTangent;

void main()
{
	vec4 worldPosition = model * vec4(inPosition, 1.0);

	vWorldPosition = worldPosition.xyz;
	vNormal = normalMatrix * inNormal;
	vTangent = vec4(mat3(model) * inTangent.xyz, inTangent.w);
	vUV = inUV;

	gl_Position = viewProjection * worldPosition;
}
