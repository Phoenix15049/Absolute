#version 330 core

in vec2 vUV;

out vec4 fragColor;

uniform sampler2D sourceTexture;
uniform bool isFirstPass;
uniform float threshold;

vec3 Prefilter(vec3 color)
{
	color = min(color, vec3(64.0));

	float brightness = max(color.r, max(color.g, color.b));
	float knee = threshold * 0.5;
	float soft = clamp(brightness - threshold + knee, 0.0, 2.0 * knee);
	soft = soft * soft / (4.0 * knee + 0.0001);

	return color * max(soft, brightness - threshold) / max(brightness, 0.0001);
}

void main()
{
	vec2 texel = 1.0 / vec2(textureSize(sourceTexture, 0));

	vec3 a = texture(sourceTexture, vUV + texel * vec2(-2.0, 2.0)).rgb;
	vec3 b = texture(sourceTexture, vUV + texel * vec2(0.0, 2.0)).rgb;
	vec3 c = texture(sourceTexture, vUV + texel * vec2(2.0, 2.0)).rgb;
	vec3 d = texture(sourceTexture, vUV + texel * vec2(-2.0, 0.0)).rgb;
	vec3 e = texture(sourceTexture, vUV).rgb;
	vec3 f = texture(sourceTexture, vUV + texel * vec2(2.0, 0.0)).rgb;
	vec3 g = texture(sourceTexture, vUV + texel * vec2(-2.0, -2.0)).rgb;
	vec3 h = texture(sourceTexture, vUV + texel * vec2(0.0, -2.0)).rgb;
	vec3 i = texture(sourceTexture, vUV + texel * vec2(2.0, -2.0)).rgb;
	vec3 j = texture(sourceTexture, vUV + texel * vec2(-1.0, 1.0)).rgb;
	vec3 k = texture(sourceTexture, vUV + texel * vec2(1.0, 1.0)).rgb;
	vec3 l = texture(sourceTexture, vUV + texel * vec2(-1.0, -1.0)).rgb;
	vec3 m = texture(sourceTexture, vUV + texel * vec2(1.0, -1.0)).rgb;

	vec3 color = e * 0.125 + (a + c + g + i) * 0.03125 + (b + d + f + h) * 0.0625 + (j + k + l + m) * 0.125;

	if (isFirstPass)
	{
		color = Prefilter(color);
	}

	fragColor = vec4(max(color, vec3(0.0001)), 1.0);
}
