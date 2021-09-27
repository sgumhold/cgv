#version 330 core

/*
The following interface is implemented in this shader:
//***** begin interface of noice_lib.glsl ***********************************
const int MAX_NUM_NOISE_LAYERS = 15;
struct NoiseLayer {
    float frequency;
    float amplitude;
    bool enabled;
};
vec3 snoise2(vec2 P);
vec4 generateHeight(in vec2 pos,
in NoiseLayer noiseLayers[MAX_NUM_NOISE_LAYERS], in int numNoiseLayers,
in bool shouldApplyPower, in float power,
in bool shouldApplyBowl, in float bowlStrength,
in bool shouldApplyPlatform, in float platformHeight,
in int seed
);
//***** end interface of noice_lib.glsl ***********************************
*/

const int MAX_NUM_NOISE_LAYERS = 15;
struct NoiseLayer {
    float frequency;
    float amplitude;
    bool enabled;
};

// https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
const float PHI = 1.61803398874989484820459;// = Golden Ratio
float gold_noise(in vec2 uv, in float seed) {
    return fract(tan(distance(uv * PHI, uv) * seed) * uv.x);
}
float gold_noise(in float seed) {
    vec2 uv = vec2(1.0);
    return fract(tan(distance(uv * PHI, uv) * seed) * uv.x);
}
float rand(vec2 uv) {
    return fract(sin(dot(uv.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

// FAST32_hash
// A very fast hashing function.  Requires 32bit support.
// http://briansharpe.wordpress.com/2011/11/15/a-fast-and-simple-32bit-floating-point-hash-function/
//
// The 2D hash formula takes the form....
// hash = mod( coord.x * coord.x * coord.y * coord.y, SOMELARGEFLOAT ) / SOMELARGEFLOAT
// We truncate and offset the domain to the most interesting part of the noise.
// SOMELARGEFLOAT should be in the range of 400.0->1000.0 and needs to be hand picked.  Only some give good results.
// A 3D hash is achieved by offsetting the SOMELARGEFLOAT value by the Z coordinate
// https://github.com/BrianSharpe/GPU-Noise-Lib/blob/master/gpu_noise_lib.glsl
void FAST32_hash_2D(vec2 gridcell, out vec4 hash_0, out vec4 hash_1) { // generates 2 random numbers for each of the 4 cell corners
    // gridcell is assumed to be an integer coordinate
    const vec2 OFFSET = vec2(26.0, 161.0);
    const float DOMAIN = 71.0;
    const vec2 SOMELARGEFLOATS = vec2(951.135664, 642.949883);
    vec4 P = vec4(gridcell.xy, gridcell.xy + 1.0);
    P = P - floor(P * (1.0 / DOMAIN)) * DOMAIN;
    P += OFFSET.xyxy;
    P *= P;
    P = P.xzxz * P.yyww;
    hash_0 = fract(P * (1.0 / SOMELARGEFLOATS.x));
    hash_1 = fract(P * (1.0 / SOMELARGEFLOATS.y));
}

// SimplexPerlin2D_Deriv
// SimplexPerlin2D noise with derivatives
// returns vec3( value, xderiv, yderiv )
// https://github.com/BrianSharpe/GPU-Noise-Lib/blob/master/gpu_noise_lib.glsl
vec3 snoise2(vec2 P) {
    //	simplex math constants
    const float SKEWFACTOR = 0.36602540378443864676372317075294;// 0.5*(sqrt(3.0)-1.0)
    const float UNSKEWFACTOR = 0.21132486540518711774542560974902;// (3.0-sqrt(3.0))/6.0
    const float SIMPLEX_TRI_HEIGHT = 0.70710678118654752440084436210485;// sqrt( 0.5 )	height of simplex triangle
    const vec3 SIMPLEX_POINTS = vec3(1.0-UNSKEWFACTOR, -UNSKEWFACTOR, 1.0-2.0*UNSKEWFACTOR);//	vertex info for simplex triangle

    //	establish our grid cell.
    P *= SIMPLEX_TRI_HEIGHT;// scale space so we can have an approx feature size of 1.0  ( optional )
    vec2 Pi = floor(P + dot(P, vec2(SKEWFACTOR)));

    //	calculate the hash.
    //	( various hashing methods listed in order of speed )
    vec4 hash_x, hash_y;
    FAST32_hash_2D(Pi, hash_x, hash_y);
    //SGPP_hash_2D( Pi, hash_x, hash_y );

    //	establish vectors to the 3 corners of our simplex triangle
    vec2 v0 = Pi - dot(Pi, vec2(UNSKEWFACTOR)) - P;
    vec4 v1pos_v1hash = (v0.x < v0.y) ? vec4(SIMPLEX_POINTS.xy, hash_x.y, hash_y.y) : vec4(SIMPLEX_POINTS.yx, hash_x.z, hash_y.z);
    vec4 v12 = vec4(v1pos_v1hash.xy, SIMPLEX_POINTS.zz) + v0.xyxy;

    //	calculate the dotproduct of our 3 corner vectors with 3 random normalized vectors
    vec3 grad_x = vec3(hash_x.x, v1pos_v1hash.z, hash_x.w) - 0.49999;
    vec3 grad_y = vec3(hash_y.x, v1pos_v1hash.w, hash_y.w) - 0.49999;
    vec3 norm = inversesqrt(grad_x * grad_x + grad_y * grad_y);
    grad_x *= norm;
    grad_y *= norm;
    vec3 grad_results = grad_x * vec3(v0.x, v12.xz) + grad_y * vec3(v0.y, v12.yw);

    //	evaluate the surflet
    vec3 m = vec3(v0.x, v12.xz) * vec3(v0.x, v12.xz) + vec3(v0.y, v12.yw) * vec3(v0.y, v12.yw);
    m = max(0.5 - m, 0.0);//	The 0.5 here is SIMPLEX_TRI_HEIGHT^2
    vec3 m2 = m*m;
    vec3 m4 = m2*m2;

    //	calc the deriv
    vec3 temp = 8.0 * m2 * m * grad_results;
    float xderiv = dot(temp, vec3(v0.x, v12.xz)) - dot(m4, grad_x);
    float yderiv = dot(temp, vec3(v0.y, v12.yw)) - dot(m4, grad_y);

    const float FINAL_NORMALIZATION = 99.204334582718712976990005025589;//	scales the final result to a strict 1.0->-1.0 range

    //	sum the surflets and return all results combined in a vec3
    return vec3(dot(m4, grad_results), xderiv, yderiv) * FINAL_NORMALIZATION;
}

void applyPower(inout vec3 noise, inout float noiseMax, in float power) {
    // f(x) = x^p
    noise.x = pow(noise.x, power);
    noiseMax = pow(noiseMax, power);

    // f(x) = g(x)^p -> f'(x) = p*g(x)^(p-1) * g'(x)
    noise.y = power * pow(noise.x, power - 1.0F) * noise.y;
    noise.z = power * pow(noise.x, power - 1.0F) * noise.z;
}

void applyBowlEffect(inout vec3 noise, inout float noiseMax, in vec2 pos, in float bowlStrength) {
    // f(x,y) = g(x,y) + ((x/500)^2 + (y/500)^2) * bS
    vec2 p = pos;
    p /= 500.0F;
    p *= p;
    noise.x += (p.x + p.y) * bowlStrength;
    noiseMax += (p.x + p.y) * bowlStrength;

    // f'(y) = g'(y) + (x/125000) * bS
    noise.y += pos.x/125000.0F * bowlStrength;
    // f'(x) = g'(x) + (y/125000) * bS
    noise.z += pos.y/125000.0F * bowlStrength;
}

void applyPlatform(inout vec3 noise, in float noiseMax, in vec2 pos, in float platformHeight) {
    float width = 1000;
    float centerX = 0.0F;
    float centerY = 0.0F;
    float platformHalf = 50.0F;
    float smoothing = 100.0F;
    float posM = 1.0F / smoothing;
    float posN = 0.0F - posM * (centerX - platformHalf - smoothing);
    float negM = -1.0F / smoothing;
    float negN = 0.0F - negM * (centerX + platformHalf + smoothing);
    float w = 1.0F;
    if (
    pos.x < centerX - platformHalf - smoothing ||
    pos.x > centerX + platformHalf + smoothing ||
    pos.y < centerY - platformHalf - smoothing ||
    pos.y > centerY + platformHalf + smoothing) {
        // outside of platform
        w = 0.0F;
    } else if (
    pos.x > centerX - platformHalf &&
    pos.x < centerX + platformHalf &&
    pos.y > centerY - platformHalf &&
    pos.y < centerY + platformHalf) {
        // inside of platform
        w = 1.0F;
    } else {
        // smoothing area
        bool isLeftEdge = pos.x <= centerX - platformHalf && pos.x >= centerX - platformHalf - smoothing;
        bool isRightEdge = pos.x >= centerX + platformHalf && pos.x <= centerX + platformHalf + smoothing;
        bool isTopEdge = pos.y >= centerY + platformHalf && pos.y <= centerY + platformHalf + smoothing;
        bool isBottomEdge = pos.y <= centerY - platformHalf && pos.y >= centerY - platformHalf - smoothing;
        if (isLeftEdge) {
            w *= posM * pos.x + posN;
        } else if (isRightEdge) {
            w *= negM * pos.x + negN;
        }
        if (isBottomEdge) {
            w *= posM * pos.y + posN;
        } else if (isTopEdge) {
            w *= negM * pos.y + negN;
        }
    }
    noise.x = (1.0F - w) * noise.x + w * platformHeight * noiseMax;
    noise.y = (1.0F - w) * noise.y;
    noise.z = (1.0F - w) * noise.z;
}

vec4 generateHeight(in vec2 pos,
in NoiseLayer noiseLayers[MAX_NUM_NOISE_LAYERS], in int numNoiseLayers,
in bool shouldApplyPower, in float power,
in bool shouldApplyBowl, in float bowlStrength,
in bool shouldApplyPlatform, in float platformHeight,
in int seed
) {
    vec3 noise = vec3(0.0F);
    float noiseMin = 0.0F;
    float noiseMax = 0.0F;

    vec2 seedOffset = vec2(gold_noise(float(seed))*10000, gold_noise(float(seed))*10000);
    for (int i = 0; i < numNoiseLayers; i++) {
        if (!noiseLayers[i].enabled) {
            continue;
        }

        float frequency = noiseLayers[i].frequency;
        float amplitude = noiseLayers[i].amplitude;
        vec2 p = pos + seedOffset;
        vec3 n = snoise2(p / frequency) * amplitude;
        n.yz /= frequency;
        noise += n;

        noiseMin -= amplitude;
        noiseMax += amplitude;
    }

    noise.x -= noiseMin;
    noiseMax -= noiseMin;

    if (shouldApplyPower) {
        applyPower(noise, noiseMax, power);
    }
    if (shouldApplyBowl) {
        applyBowlEffect(noise, noiseMax, pos, bowlStrength);
    }
    if (shouldApplyPlatform) {
        applyPlatform(noise, noiseMax, pos, platformHeight);
    }

    float normalizedHeight = noise.x / noiseMax;
    vec4 result = vec4(noise, normalizedHeight);
    return result;
}

