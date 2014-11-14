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
    float freq = texture2D(fft, vec2(p.x, 0.0)).r ;

    gl_FragColor = vec4(0.0, freq, freq, 0.0);
}
