#version 330 core

in vec2 vUV;

out vec4 fragColor;

uniform sampler2D inputTexture;

const float ReduceMin = 1.0 / 128.0;
const float ReduceMultiplier = 1.0 / 8.0;
const float SpanMax = 8.0;

void main()
{
	vec2 texel = 1.0 / vec2(textureSize(inputTexture, 0));

	float lumaNW = texture(inputTexture, vUV + vec2(-1.0, -1.0) * texel).a;
	float lumaNE = texture(inputTexture, vUV + vec2(1.0, -1.0) * texel).a;
	float lumaSW = texture(inputTexture, vUV + vec2(-1.0, 1.0) * texel).a;
	float lumaSE = texture(inputTexture, vUV + vec2(1.0, 1.0) * texel).a;
	vec4 center = texture(inputTexture, vUV);
	float lumaM = center.a;

	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	vec2 direction = vec2(-((lumaNW + lumaNE) - (lumaSW + lumaSE)), (lumaNW + lumaSW) - (lumaNE + lumaSE));

	float reduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.25 * ReduceMultiplier, ReduceMin);
	float inverseMin = 1.0 / (min(abs(direction.x), abs(direction.y)) + reduce);
	direction = clamp(direction * inverseMin, vec2(-SpanMax), vec2(SpanMax)) * texel;

	vec3 colorA = 0.5 * (texture(inputTexture, vUV + direction * (1.0 / 3.0 - 0.5)).rgb +
	                     texture(inputTexture, vUV + direction * (2.0 / 3.0 - 0.5)).rgb);

	vec3 colorB = colorA * 0.5 + 0.25 * (texture(inputTexture, vUV - direction * 0.5).rgb +
	                                     texture(inputTexture, vUV + direction * 0.5).rgb);

	float lumaB = dot(colorB, vec3(0.299, 0.587, 0.114));

	fragColor = vec4((lumaB < lumaMin || lumaB > lumaMax) ? colorA : colorB, 1.0);
}
