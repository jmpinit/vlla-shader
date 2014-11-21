#version 100
#ifdef GL_ES
precision mediump float;
#endif

#define PI 3.1415

uniform sampler2D fft;
uniform sampler2D last;

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

    return vec4(rgb, 1.0);
}

float snd(float i) {
    return texture2D(fft, vec2(i, 0.0)).r ;
}

vec4 pixel(vec2 pixel) {
    vec2 p = pixel / resolution.xy;
    float bass = snd(0.0) + snd(0.025) + snd(0.05) + snd(0.075);
    float bassAngle = bass * 64.0;
    vec2 center = vec2(0.5, 0.5);
    float angle = atan(p.y - center.y, p.x - center.x);
    float normAngle = angle / (2.0 * PI);
    float freq = snd(normAngle);

    float angleMod = angle - PI / 32.0;
    vec2 prevPos = center + (distance(center, p) - 2.5 / resolution.x) * vec2(cos(angleMod), sin(angleMod));
    vec4 prev = texture2D(last, prevPos) ;
    
    float inCircle = step(distance(center, p), 0.1) ;
    float outCircle = 1.0 - inCircle;

    return vec4(0.0, 0.5, 1.0 + bass * 32.0, 0.0) * inCircle * freq * 8.0 + outCircle * prev;
}

void main() {
    vec2 p = gl_FragCoord.xy / resolution.xy;
    vec2 center = vec2(0.5, 0.5);

    float angle = atan(p.y - center.y, p.x - center.x);
    float normAngle = angle / (2.0 * PI);
    float freq = 0.5 * snd(normAngle);

    float inCircle = step(distance(center, p), 0.13) ;
    float outCircle = 1.0 - inCircle;

    vec4 laplace = pixel(gl_FragCoord.xy + vec2(0.0, 1.0)) +
        pixel(gl_FragCoord.xy + vec2(0.0, -1.0)) +
        pixel(gl_FragCoord.xy + vec2(1.0, 0.0)) +
        pixel(gl_FragCoord.xy + vec2(-1.0, 0.0)) +
        - 4.0 * pixel(gl_FragCoord.xy);

    vec4 centerPart = vec4(0.0, 0.5, 1.0, 0.0) * inCircle * freq * 8.0;
    gl_FragColor = centerPart + outCircle * 0.4 * laplace;
}
