#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

in vec3 fragPos;

out vec4 fColor;

//lighting
uniform mat3 lightDirMatrix;
uniform	mat3 normalMatrix;
uniform	vec3 lightDir;
uniform	vec3 lightColor;

// fog
uniform int foginit;
uniform float fogDensity;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float constant = 1.0f;
float linear = 0.00225f;
float quadratic = 0.00375;

uniform mat4 view;

// point light
uniform int pointinit;

uniform vec3 lightPos1;
uniform vec3 lightPos2;

float ambientPoint = 0.5f;
float specularStrengthPoint = 0.5f;
float shininessPoint = 32.0f;

float computeShadow()
{

	// perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	if(normalizedCoords.z > 1.0f)
			return 0.0f;

	// Transform to [0,1] range
	normalizedCoords = normalizedCoords * 0.5f + 0.5f;

  // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

  // Get depth of current fragment from light's perspective
	float currentDepth = normalizedCoords.z;

  // Check whether current frag pos is in shadow
	float bias = 0.005f;
	float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;

	return shadow;
}

float computeFog(){

  float fragmentDistance = length(fPosEye);
  float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
  return clamp(fogFactor, 0.0f, 1.0f);
}

//point light
vec3 computePointLight(vec4 lightPosEye)
{
	vec3 cameraPosEye = vec3(0.0f);
	vec3 normalEye = normalize(normalMatrix * fNormal);
	vec3 lightDirN = normalize(lightPosEye.xyz - fPosEye.xyz);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 ambient = ambientPoint * lightColor;
	vec3 diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	vec3 halfVector = normalize(lightDirN + viewDirN);
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininessPoint);
	vec3 specular = specularStrengthPoint * specCoeff * lightColor;
	float distance = length(lightPosEye.xyz - fPosEye.xyz);
	float att = 1.0f / (constant + linear * distance + quadratic * distance * distance);
	return (ambient + diffuse + specular) * att * vec3(2.0f,2.0f,2.0f);
}

vec3 computeLightComponents()
{
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin

	//transform normal
	vec3 normalEye = normalize(fNormal);

	//compute light direction
	vec3 lightDirN = normalize(lightDir);

	//compute view direction
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

	//compute ambient light
	ambient = ambientStrength * lightColor;

	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;

	return (ambient + diffuse + specular);
}



void main()
{
	vec3 light = computeLightComponents();

	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f);//orange

	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	float shadow = computeShadow();
	vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);

	fColor = vec4(color, 1.0f);

        float fogFactor = computeFog();
        vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

	// pointlight

	if (pointinit == 1 ){
	vec4 lightPosEye1 = view * vec4(lightPos1, 1.0f);
	light += computePointLight(lightPosEye1);

	vec4 lightPosEye2 =  view * vec4(lightPos2, 1.0f);
	light += computePointLight(lightPosEye2);

	vec4 diffuseColor = texture(diffuseTexture, fTexCoords);
	}

        vec4 colorWithShadow = vec4(color, 1.0f);
	if(foginit == 0)
	{
		fColor = min(colorWithShadow * vec4(light, 1.0f), 1.0f);

	}
	else
	{
		fColor = mix(fogColor, min(colorWithShadow * vec4(light, 1.0f), 1.0f), fogFactor);
	}

}
