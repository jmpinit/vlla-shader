#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D fft;

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

void main() {
    gl_FragColor = texture2D(fft, vec2(gl_FragCoord.x / resolution.x, 0.0));
}
