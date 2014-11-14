#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D fft;
uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

void main( void ) {
    vec2 p = gl_FragCoord.xy / resolution.xy ;
    float freq = texture2D(fft, vec2(distance(vec2(0.5, 0.5), p), 0.0)).r * 15.0;

    float color = 0.0;
    color += sin( p.x * cos( time / 15.0 ) * 80.0 ) + cos( p.y * cos( time / 15.0 ) * 10.0 );
    color += sin( p.y * sin( time / 10.0 ) * 40.0 ) + cos( p.x * sin( time / 25.0 ) * 40.0 );
    color += sin( p.x * sin( time / 5.0 ) * 10.0 ) + sin( p.y * sin( time / 35.0 ) * 80.0 );
    color *= sin( time / 10.0 ) * freq;

    gl_FragColor = vec4( vec3( color, color * freq, freq * sin( color + time / 3.0 ) * 0.75 ), 1.0 );
}
