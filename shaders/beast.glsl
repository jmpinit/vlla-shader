#version 100
#ifdef GL_ES
precision mediump float;
#endif

#define PI 3.1415926

uniform sampler2D fft;
uniform sampler2D last;

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;
uniform vec2 range;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 color(float n) {
    float h = 3.1415*cos(n);
    float v = sqrt(n/10.0);
    float s = 1.0 - v;

    vec3 rgb = (hsv2rgb(vec3(h,s,v)));

    return vec4(rgb,1.0);
}

float snd(float i) {
    return texture2D(fft, vec2(i, 0.0)).r ;
}

float fuzzsnd(float i, float d) {
    return snd(i-d) + snd(i) + snd(i+d);
}

float inEye(vec2 p, vec2 pos, float opening) {
    return step(sqrt(pow(p.x-pos.x, 2.0) + pow(p.y-pos.y, opening)), 0.1) ;
}

vec4 eye(vec2 pos, vec2 look) {
    vec2 p = gl_FragCoord.xy / resolution.xy;
    float bass = snd(0.0) + snd(0.025) + snd(0.05) + snd(0.075);

    float opening = clamp(1.8 + 32.0 * bass, 1.5, 3.0) ;
    float iris = 1.0 - step(distance(p, pos + look), 0.05);

    return vec4(0.0, 0.0, iris, 0.0) * inEye(p, pos, opening);
}

vec4 hair() {
    vec2 p = gl_FragCoord.xy / resolution.xy;
    float bass = snd(0.0) + snd(0.025) + snd(0.05) + snd(0.075);
    float fineness = 0.05;
    float flow = p.x * (1.0 - p.y) + p.y * 0.5;
    return vec4(0.0, 0.02, 0.1, 0.0) * pow(mod(flow, fineness) - fineness / 2.0, 2.0) * 400.0 ;
}

void main() {
    vec2 center = vec2(0.5 + 0.1 * sin(time / 30.0), 0.5);
    vec2 p = gl_FragCoord.xy / resolution.xy;

    float bass = snd(0.0) + snd(0.025) + snd(0.05) + snd(0.075);
    vec2 look = vec2(0.05 * sin(bass * 32.0 + time / 30.0), 0.0);
    vec2 leftEye = vec2(-0.25, 0.0);
    vec2 rightEye = vec2(0.25, 0.0);

    float eyeSpread = ((rightEye - leftEye)/2.0).x;
    float overEye = step(eyeSpread - 0.1, distance(p.x, center.x)) ;
    float xPosInEye = (distance(p.x, center.x) - eyeSpread) / 0.1;
    float eyefft = 32.0 * snd(xPosInEye);
    vec4 prev = ((0.2 + eyefft) * step(0.05, p.y) * texture2D(last, p - vec2(0.0, 0.05))).bbba;
    float flameOn = step(0.1, range.y) ;

    gl_FragColor = eye(center + leftEye, look) + eye(center + rightEye, look) + vec4(0.0, 0.5, 1.0, 0.0) * prev * flameOn;
}
