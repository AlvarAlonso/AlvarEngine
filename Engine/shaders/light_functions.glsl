vec3 light_position = vec3(0.0, 10.0, 4.0);
vec3 light_color = vec3(1.0, 1.0, 1.0);

vec3 Phong(in vec3 aNormal, in vec3 aPointPosition)
{
    vec3 N = normalize(aNormal);
    vec3 L = normalize(light_position - aPointPosition);
    float NdotL = dot(N, L);
    NdotL = clamp(NdotL, 0.0, 1.0);
    return NdotL * light_color;
}