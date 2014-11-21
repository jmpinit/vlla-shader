#version 100
#ifdef GL_ES
precision mediump float;
#endif

#define PI 3.1415926

uniform sampler2D fft;

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

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

void main() {
    vec2 p = gl_FragCoord.xy / resolution.xy;
    vec2 center = vec2(0.5, 0.5);

    float bass = snd(0.0) + snd(0.025) + snd(0.5) + snd(1.0);
    vec2 fromCenter = center - p;

    gl_FragColor = color(16.0 * fuzzsnd(0.2 + (p.x - sin(distance(p, vec2(bass, 0.5)))), 0.05)) ;
}
