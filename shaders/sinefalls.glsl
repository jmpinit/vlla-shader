#version 100
#ifdef GL_ES
precision mediump float;
#endif

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

void main() {
    vec2 p = gl_FragCoord.xy / resolution.xy;
    float freq = texture2D(fft, vec2(abs(p.x + 0.1 * cos(4.0 * 3.1415*p.y) - 0.5), 0.0)).r ;

    gl_FragColor = color(20.0*freq);
}
