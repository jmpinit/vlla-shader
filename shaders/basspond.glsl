#version 100
#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D fft;

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

void main() {
    vec2 p = gl_FragCoord.xy / resolution.xy;
    float d = sqrt(pow(p.x-0.5, 2.0) + pow(p.y-0.5, 2.0));
    float freq = texture2D(fft, vec2(0.05 * d / sqrt(2.0) + 0.1 , 0.0)).r;

    float c = 1.0 * freq;
    gl_FragColor = vec4(c, c, c, 0.0);
}
