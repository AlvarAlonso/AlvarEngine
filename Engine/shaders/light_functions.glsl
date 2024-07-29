#define RECIPROCAL_PI 0.3183098861837697
#define PI 3.1415926535897932384626433832795

vec3 light_position = vec3(0.0, 10.0, 4.0);
vec3 light_color = vec3(1.0, 1.0, 1.0);

// TODO: Place the real world_position of the fragment shader.
vec3 world_position = vec3(0.0, 0.0, 0.0);

vec3 Phong(in vec3 aNormal, in vec3 aPointPosition)
{
    vec3 N = normalize(aNormal);
    vec3 L = normalize(light_position - aPointPosition);
    float NdotL = dot(N, L);
    NdotL = clamp(NdotL, 0.0, 1.0);
    return NdotL * light_color;
}

float GetSpotFactor( in float aCosine, in float aExponent)
{
	return pow(aCosine, aExponent);
}

vec3 SpotDirection( in float aSpotCosine, in vec3 aSpotDirection, in vec3 aLightPosition, 
    in vec3 aWorldPosition, in vec3 aNormal, in vec3 aLightColor, in float aSpotExponent, in float aIntensity )
{
	vec3 L = normalize( aLightPosition - aWorldPosition );
	float theta = dot( -L, normalize(aSpotDirection) );
	if(theta >= aSpotCosine)
	{
		vec3 final_color = Phong( aNormal, world_position ) * GetSpotFactor( aSpotCosine, aSpotExponent );
		return final_color;
	}	
	return vec3(0.0);
}

float ComputeAttenuation( in vec3 aLightPosition, in vec3 aObjectPosition, in float aMaxDist )
{
	float distance = length(aObjectPosition - aLightPosition);
	float att_factor = aMaxDist - distance;
	att_factor /= aMaxDist;
	att_factor = max(att_factor, 0.0);
	return att_factor*att_factor;
}

float D_GGX ( const in float NoH, const in float aLinearRoughness )
{
	float a2 = aLinearRoughness * aLinearRoughness;
	float f = (NoH * NoH) * (a2 - 1.0) + 1.0;
	return a2 / (PI * f * f);
}

vec3 F_Schlick( const in float VoH, const in vec3 f0 )
{
	float f = pow(1.0 - VoH, 5.0);
	return f0 + (vec3(1.0) - f0) * f;
}

float GGX(float NdotV, float k)
{
	return NdotV / (NdotV * (1.0 - k) + k);
}
	
float G_Smith( float NdotV, float NdotL, float aRoughness)
{
	float k = pow(aRoughness + 1.0, 2.0) / 8.0;
	return GGX(NdotL, k) * GGX(NdotV, k);
}

vec3 SpecularBRDF( float aRoughness, vec3 f0, float NoH, float NoV, float NoL, float LoH )
{
	float a = aRoughness * aRoughness;

	float D = D_GGX( NoH, a );
	vec3 F = F_Schlick( LoH, f0 );
	float G = G_Smith( NoV, NoL, aRoughness );
	
	vec3 spec = D * G * F;
	spec /= ( 4.0 * NoL * NoV + 1e-6 );

	return spec;
}

vec3 Degamma(vec3 c)
{
	return pow(c,vec3(2.2));
}

vec3 Gamma(vec3 c)
{
	return pow(c,vec3(1.0/2.2));
}

float Fd_Burley ( const in float NoV, const in float NoL,
				const in float LoH, 
				const in float aLinearRoughness)
{
        float f90 = 0.5 + 2.0 * aLinearRoughness * LoH * LoH;
        float lightScatter = F_Schlick( NoL, vec3(1.0) ).x;
        float viewScatter  = F_Schlick( NoV, vec3(1.0) ).x;
        return lightScatter * viewScatter * RECIPROCAL_PI;
}