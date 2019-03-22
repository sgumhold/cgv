uniform sampler2D depth_texture;
uniform float     depth_scale;

uniform sampler2D color_texture;
uniform float     color_scale;


float RawDepthToMeters(float depthValue)
{
    if (depthValue < 0.03125) {
        return 1.0 / (depthValue * -201.2677144576 + 3.3309495161);
    }
	else {
		return 0.0;
	}
}

vec3 DepthToWorld(vec2 tc, float depthValue)
{
    vec3 result;
    float depth = RawDepthToMeters(depthValue);
    result.x = float((tc.x - 0.5301684528) * depth * 1.077052428);
    result.y = float((tc.y - 0.5057065367) * depth * 0.8121270369);
    result.z = depth;
    return result;
}

vec2 WorldToPixel(vec3 pt)
{
	float invZ = 1.0 / pt.z;

	vec2 result;
    result.x = pt.x*0.8268985641*invZ + 0.5139730005;
    result.y = pt.y*1.094924867 *invZ + 0.5572514202;
    return result;
}
