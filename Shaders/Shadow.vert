#version 330 core

layout(location = 0) in vec3 inPosition;

uniform mat4 lightSpaceModel;

void main()
{
	gl_Position = lightSpaceModel * vec4(inPosition, 1.0);
}
