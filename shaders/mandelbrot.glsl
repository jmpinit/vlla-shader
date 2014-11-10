precision mediump float;

uniform float iGlobalTime;
uniform vec2 iResolution;

#define CENTER vec2(0.5,0.5)
#define ITER 50
#define SCALE 2.0

#define PI 3.14159

float radius(float t) 
{
    return abs(2.0-mod(t,4.0));
}

vec2 complexSquare(vec2 c) 
{
    vec2 result;
    result.x = c.x * c.x - c.y * c.y;
    result.y = 2.0 * c.x * c.y;
    return result;
}

vec2 seed(float t,float factor)
{
    float r = radius(t/(2.0*PI*factor));
    return r*vec2(cos(t),sin(t));
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 color(float n)
{
    float h = PI*cos(n+iGlobalTime/10.0);
    float v = sqrt(n/float(ITER));
    float s = 1.0 - v;

    vec3 rgb = (hsv2rgb(vec3(h,s,v)));

    return vec4(rgb,1.0);
}


void main(void)
{
    vec2 uv = gl_FragCoord.xy / vec2(60.0, 32.0);
    float proportion = 60.0/32.0;

    vec2 coords = uv - CENTER;
    vec2 z = SCALE * vec2(proportion*coords.x, coords.y);

    int iteration;

    for(int i = 0; i < ITER; i++) {
        z = complexSquare(z) + seed(iGlobalTime,3.0);
        iteration = i;
        if(length(z) > 4.0) break;
    }

    gl_FragColor = color(float(iteration));
}
