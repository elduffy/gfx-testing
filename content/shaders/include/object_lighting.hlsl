#define MAX_NUM_LIGHTS 8

#define OBJECT_LIGHTING(Name, n) UNIFORM_BUFFER_FRAG(Name, n) \
{ \
    float3 cameraPosMS; \
    float pad; \
    uint numLights; \
    float3 lightPosMS[MAX_NUM_LIGHTS]; \
};

#define FOR_EACH_LIGHT(i) for (int i = 0; i < min(numLights, MAX_NUM_LIGHTS); i++)