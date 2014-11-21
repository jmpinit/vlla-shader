#version 100
#ifdef GL_ES
precision mediump float;
#endif

#define PI 3.1415926

uniform sampler2D fft;

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

float snd(float i) {
    return texture2D(fft, vec2(i, 0.0)).r ;
}

void main() {
    vec2 p = gl_FragCoord.xy / resolution.xy;
    vec2 center = vec2(0.5, 0.5);

    float bass = snd(0.0) + snd(0.025) + snd(0.5) + snd(1.0);
    float freq = bass;
    vec2 fromCenter = center - p;
    float angle = atan(fromCenter.y, fromCenter.x) + PI ;
    float inCircle = step(distance(p, center), 0.4);

    float spiral = sin((0.05 + bass) * 128.0 * 3.1415 * distance(p, center) / sqrt(2.0)) ;

    float c = spiral;
    gl_FragColor = 0.05 * vec4(sin(time / 10.0) * c, cos(time / 4.0) * c, sin(time / 13.0) * c, 0.0) ;
}
