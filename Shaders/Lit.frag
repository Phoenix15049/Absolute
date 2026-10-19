#version 330 core

#include "Common.glsl"

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 32
#define MAX_SPOT_LIGHTS 16

struct DirectionalLight
{
	vec4 direction;
	vec4 color;
};

struct PointLight
{
	vec4 positionRange;
	vec4 colorIntensity;
};

struct SpotLight
{
	vec4 positionRange;
	vec4 directionOuter;
	vec4 colorIntensity;
	vec4 innerCone;
};

layout(std140) uniform LightData
{
	ivec4 lightCounts;
	DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
	PointLight pointLights[MAX_POINT_LIGHTS];
	SpotLight spotLights[MAX_SPOT_LIGHTS];
};

struct Material
{
	vec3 diffuse;
	vec3 specular;
	vec3 emissive;
	float shininess;
	float opacity;
	float normalStrength;
};

in vec3 vWorldPosition;
in vec3 vNormal;
in vec2 vUV;
in vec4 vTangent;

out vec4 fragColor;

uniform Material material;
uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform sampler2D normalMap;
uniform sampler2DShadow shadowMap;
uniform bool isTransparent;
uniform vec3 highlightColor;

float Attenuation(float distance, float range)
{
	float ratio = distance / max(range, 0.0001);
	float window = clamp(1.0 - ratio * ratio * ratio * ratio, 0.0, 1.0);
	return (window * window) / (distance * distance + 1.0);
}

vec3 Shade(vec3 lightDirection, vec3 radiance, vec3 normal, vec3 viewDirection, vec3 albedo, vec3 specularColor)
{
	float diffuseTerm = max(dot(normal, lightDirection), 0.0);

	if (diffuseTerm <= 0.0)
	{
		return vec3(0.0);
	}

	vec3 halfway = normalize(lightDirection + viewDirection);
	float shininess = max(material.shininess, 1.0);
	float normalization = (shininess + 8.0) / 25.13274;
	float specularTerm = pow(max(dot(normal, halfway), 0.0), shininess) * normalization;

	return radiance * diffuseTerm * (albedo + specularColor * specularTerm);
}

float ShadowFactor(vec3 normal, vec3 lightDirection)
{
	if (shadowParams.x < 0.5)
	{
		return 1.0;
	}

	float slope = 1.0 - max(dot(normal, lightDirection), 0.0);
	vec3 offsetPosition = vWorldPosition + normal * shadowParams.y * (0.5 + slope);
	vec4 shadowCoord = lightSpace * vec4(offsetPosition, 1.0);
	vec3 coord = shadowCoord.xyz / shadowCoord.w * 0.5 + 0.5;

	if (coord.z > 1.0)
	{
		return 1.0;
	}

	int radius = int(shadowParams.z);
	vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
	float total = 0.0;
	float samples = 0.0;

	for (int x = -radius; x <= radius; x++)
	{
		for (int y = -radius; y <= radius; y++)
		{
			total += texture(shadowMap, vec3(coord.xy + vec2(x, y) * texelSize, coord.z - 0.0005));
			samples += 1.0;
		}
	}

	return total / samples;
}

void main()
{
	vec4 diffuseSample = texture(diffuseMap, vUV);
	float alpha = diffuseSample.a * material.opacity;

	if (!isTransparent && alpha < 0.5)
	{
		discard;
	}

	vec3 albedo = material.diffuse * diffuseSample.rgb;
	vec3 specularColor = material.specular * texture(specularMap, vUV).rgb;

	vec3 normal = normalize(vNormal);

	if (!gl_FrontFacing)
	{
		normal = -normal;
	}

	vec3 tangent = vTangent.xyz - normal * dot(normal, vTangent.xyz);

	if (dot(tangent, tangent) > 1e-8)
	{
		tangent = normalize(tangent);
		vec3 bitangent = cross(normal, tangent) * vTangent.w;
		vec3 mapped = texture(normalMap, vUV).xyz * 2.0 - 1.0;
		mapped.xy *= material.normalStrength;
		normal = normalize(mat3(tangent, bitangent, normal) * mapped);
	}

	int renderMode = options.x;

	if (renderMode == 1)
	{
		fragColor = vec4(albedo, alpha);
		return;
	}

	if (renderMode == 2)
	{
		fragColor = vec4(normal * 0.5 + 0.5, 1.0);
		return;
	}

	if (renderMode == 3)
	{
		albedo = vec3(0.8);
		specularColor = vec3(0.04);
	}

	vec3 viewDirection = normalize(cameraPosition.xyz - vWorldPosition);

	float hemisphere = normal.y * 0.5 + 0.5;
	vec3 skyAmbient = mix(skyHorizon.rgb, skyTop.rgb, 0.15 * max(normal.y, 0.0));
	vec3 ambient = mix(groundColor.rgb, skyAmbient, hemisphere) * skyHorizon.w;
	vec3 color = ambient * albedo;

	for (int i = 0; i < lightCounts.x; i++)
	{
		vec3 lightDirection = -normalize(directionalLights[i].direction.xyz);
		vec3 radiance = directionalLights[i].color.rgb * directionalLights[i].color.w;
		float shadow = (directionalLights[i].direction.w > 0.5) ? ShadowFactor(normal, lightDirection) : 1.0;

		color += Shade(lightDirection, radiance, normal, viewDirection, albedo, specularColor) * shadow;
	}

	for (int i = 0; i < lightCounts.y; i++)
	{
		vec3 toLight = pointLights[i].positionRange.xyz - vWorldPosition;
		float distance = length(toLight);
		float attenuation = Attenuation(distance, pointLights[i].positionRange.w);

		if (attenuation > 0.0)
		{
			vec3 radiance = pointLights[i].colorIntensity.rgb * pointLights[i].colorIntensity.w * attenuation;
			color += Shade(toLight / distance, radiance, normal, viewDirection, albedo, specularColor);
		}
	}

	for (int i = 0; i < lightCounts.z; i++)
	{
		vec3 toLight = spotLights[i].positionRange.xyz - vWorldPosition;
		float distance = length(toLight);
		vec3 lightDirection = toLight / distance;

		float cosAngle = dot(-lightDirection, normalize(spotLights[i].directionOuter.xyz));
		float cone = smoothstep(spotLights[i].directionOuter.w, spotLights[i].innerCone.x, cosAngle);
		float attenuation = Attenuation(distance, spotLights[i].positionRange.w) * cone;

		if (attenuation > 0.0)
		{
			vec3 radiance = spotLights[i].colorIntensity.rgb * spotLights[i].colorIntensity.w * attenuation;
			color += Shade(lightDirection, radiance, normal, viewDirection, albedo, specularColor);
		}
	}

	color += material.emissive;

	float fresnel = pow(1.0 - max(dot(normal, viewDirection), 0.0), 3.0);
	color += highlightColor * (0.15 + fresnel * 0.85);

	fragColor = vec4(color, alpha);
}
