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

    float c = 0.5 + 0.5 * sin(4.0 * time);
    gl_FragColor = vec4(c, c, c, 0.0);
}
