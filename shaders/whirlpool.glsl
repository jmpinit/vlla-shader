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
    vec2 fromCenter = center - p;
    float angle = atan(fromCenter.y, fromCenter.x) + PI + sin(2.0 * distance(p, center));
    float inCircle = step(distance(p, center), 0.4);

    float fIndex = 2.0 * angle / (2.0 * PI) + PI / 2.0;
    float fw = 0.05;
    //float freq = 8.0 * (snd(fIndex-fw) + snd(fIndex) + snd(fIndex+fw));
    float freq = 2.0 * snd(fIndex-fw);

    gl_FragColor = freq * vec4((sin(time / 31.0)+1.0), cos(time / 19.0), sin(time / 13.0), 0.0) * (bass + 1.0);
}
