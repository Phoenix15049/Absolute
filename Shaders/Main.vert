#version 460

in vec3 vertexIn; 
in vec3 colorIn;
in vec2 textureIn;

out vec3 colorOut;
out vec2 textureOut;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;


void main(){

	colorOut = colorIn;
	textureOut = textureIn;
	gl_Position = proj * view * model * vec4(vertexIn,1.0);

}