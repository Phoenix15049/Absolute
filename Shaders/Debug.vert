#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

uniform mat4 viewProjection;
uniform float pointSize;

out vec4 vColor;

void main()
{
	vColor = inColor;
	gl_PointSize = pointSize;
	gl_Position = viewProjection * vec4(inPosition, 1.0);
}
