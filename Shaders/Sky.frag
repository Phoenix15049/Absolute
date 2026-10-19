#version 330 core

#include "Common.glsl"

in vec2 vUV;

out vec4 fragColor;

uniform vec3 sunDirection;
uniform vec3 sunColor;

void main()
{
	vec4 farPoint = inverseViewProjection * vec4(vUV * 2.0 - 1.0, 1.0, 1.0);
	vec3 direction = normalize(farPoint.xyz / farPoint.w - cameraPosition.xyz);

	float height = direction.y;
	vec3 sky = mix(skyHorizon.rgb, skyTop.rgb, pow(clamp(height, 0.0, 1.0), 0.6));
	vec3 ground = mix(skyHorizon.rgb, groundColor.rgb, pow(clamp(-height * 4.0, 0.0, 1.0), 0.5));
	vec3 color = (height >= 0.0) ? sky : ground;

	float sun = max(dot(direction, -sunDirection), 0.0);
	color += sunColor * (pow(sun, 1200.0) * 40.0 + pow(sun, 24.0) * 0.15);

	fragColor = vec4(color * skyTop.w, 1.0);
}
