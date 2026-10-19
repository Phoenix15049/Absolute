#version 330 core

in vec2 vUV;

out vec4 fragColor;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;

uniform int toneMapping;
uniform float exposure;
uniform float gamma;
uniform bool isBloomEnabled;
uniform float bloomIntensity;
uniform bool isVignetteEnabled;
uniform bool isLumaOutput;

vec3 ACES(vec3 color)
{
	const mat3 inputMatrix = mat3(0.59719, 0.07600, 0.02840,
	                              0.35458, 0.90834, 0.13383,
	                              0.04823, 0.01566, 0.83777);

	const mat3 outputMatrix = mat3(1.60475, -0.10208, -0.00327,
	                               -0.53108, 1.10813, -0.07276,
	                               -0.07367, -0.00605, 1.07602);

	color = inputMatrix * color;
	vec3 a = color * (color + 0.0245786) - 0.000090537;
	vec3 b = color * (0.983729 * color + 0.4329510) + 0.238081;
	return clamp(outputMatrix * (a / b), 0.0, 1.0);
}

float Random(vec2 coord)
{
	return fract(sin(dot(coord, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
	vec3 color = texture(sceneTexture, vUV).rgb;

	if (isBloomEnabled)
	{
		color += texture(bloomTexture, vUV).rgb * bloomIntensity;
	}

	color *= exposure;

	if (toneMapping == 1)
	{
		color = color / (color + 1.0);
	}

	else if (toneMapping == 2)
	{
		color = ACES(color);
	}

	if (isVignetteEnabled)
	{
		vec2 offset = vUV - 0.5;
		color *= 1.0 - dot(offset, offset) * 0.45;
	}

	color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / gamma));
	color += (Random(gl_FragCoord.xy) - 0.5) / 255.0;

	float luma = dot(color, vec3(0.299, 0.587, 0.114));
	fragColor = vec4(color, isLumaOutput ? luma : 1.0);
}
