#version 330 core

in vec2 vUV;

out vec4 fragColor;

uniform sampler2D sourceTexture;
uniform float radius;

void main()
{
	vec2 offset = radius / vec2(textureSize(sourceTexture, 0));

	vec3 color = texture(sourceTexture, vUV).rgb * 4.0;
	color += (texture(sourceTexture, vUV + vec2(-offset.x, 0.0)).rgb +
	          texture(sourceTexture, vUV + vec2(offset.x, 0.0)).rgb +
	          texture(sourceTexture, vUV + vec2(0.0, -offset.y)).rgb +
	          texture(sourceTexture, vUV + vec2(0.0, offset.y)).rgb) * 2.0;
	color += texture(sourceTexture, vUV + vec2(-offset.x, -offset.y)).rgb +
	         texture(sourceTexture, vUV + vec2(offset.x, -offset.y)).rgb +
	         texture(sourceTexture, vUV + vec2(-offset.x, offset.y)).rgb +
	         texture(sourceTexture, vUV + vec2(offset.x, offset.y)).rgb;

	fragColor = vec4(color / 16.0, 1.0);
}
