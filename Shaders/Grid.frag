#version 330 core

#include "Common.glsl"

in vec3 vWorldPosition;

out vec4 fragColor;

uniform float gridExtent;

float GridLine(vec2 coord, float spacing)
{
	vec2 scaled = coord / spacing;
	vec2 derivative = fwidth(scaled);
	vec2 grid = abs(fract(scaled - 0.5) - 0.5) / derivative;
	return 1.0 - min(min(grid.x, grid.y), 1.0);
}

void main()
{
	vec2 coord = vWorldPosition.xz;

	float heightFade = clamp(abs(cameraPosition.y) / 50.0, 0.0, 1.0);
	float minor = GridLine(coord, 1.0) * 0.35 * (1.0 - heightFade);
	float major = GridLine(coord, 10.0) * 0.7;
	float alpha = max(minor, major);

	vec3 color = vec3(0.55);
	vec2 derivative = fwidth(coord);

	if (abs(coord.x) < derivative.x * 1.5)
	{
		color = vec3(0.25, 0.45, 1.0);
		alpha = 0.9;
	}

	if (abs(coord.y) < derivative.y * 1.5)
	{
		color = vec3(1.0, 0.3, 0.3);
		alpha = 0.9;
	}

	float distance = length(vWorldPosition - cameraPosition.xyz);
	float fade = 1.0 - smoothstep(gridExtent * 0.3, gridExtent * 0.95, distance);

	fragColor = vec4(color, alpha * fade);

	if (fragColor.a < 0.01)
	{
		discard;
	}
}
