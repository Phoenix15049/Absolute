#version 330 core

in vec4 vColor;

out vec4 fragColor;

uniform bool isPoint;

void main()
{
	if (isPoint)
	{
		vec2 offset = gl_PointCoord * 2.0 - 1.0;
		float distance = dot(offset, offset);

		if (distance > 1.0)
		{
			discard;
		}

		float ring = smoothstep(0.55, 0.7, distance);
		fragColor = vec4(mix(vColor.rgb, vec3(0.05), ring), vColor.a);
		return;
	}

	fragColor = vColor;
}
