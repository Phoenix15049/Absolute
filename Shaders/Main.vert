#version 460

in vec3 vertexIn; 
in vec3 colorIn;
out vec3 colorOut;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main(){

	colorOut = colorIn;
	gl_Position = proj * view * model * vec4(vertexIn,1.0);

}