#version 100
#ifdef GL_ES
precision mediump float;
#endif

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

    return vec4(rgb,1.0);
}

float snd(float i) {
    return texture2D(fft, vec2(i, 0.0)).r ;
}

void main() {
    vec2 p = gl_FragCoord.xy / resolution.xy;
    float freq = texture2D(fft, vec2(p.y, 0.0)).r ;
    float pixel = 1.0 / resolution.x;

    vec4 prevl = texture2D(last, p - vec2(0.5 * pixel, -pixel)) ;
    vec4 prevm = texture2D(last, p - vec2(0.0, -pixel)) ;
    vec4 prevr = texture2D(last, p + vec2(0.5 * pixel, -pixel)) ;

    float bass = snd(0.0) + snd(0.025) + snd(0.05) + snd(0.075) + snd(0.01) ;
    
    float first = step(p.x, 0.5 / resolution.x);
    float rest = 1.0 - first;

    gl_FragColor = vec4(0.0, 0.5, 1.0, 0.0) * first * freq * 4.0 + rest * ((bass * 3.0 + 0.3) * prevl + 0.3 * prevm + 0.0 * prevr);
}
