#ifdef GL_ES
precision mediump float;
#endif

// I am so sorry

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

void main( void ) {

    vec2 p = ( gl_FragCoord.xy / resolution.xy );
    p = 2.0 * p - 1.0;
    p.x *= resolution.x / resolution.y;

    p *= 1.0 + 0.9 * sin(time * 50.0) + 0.17 * cos(time * 108.3);

    vec2 q = p;
    mat2 m = mat2(cos(time * 4.0), -sin(time * 14.0), sin(time * 14.0), cos(time * 14.0));
    p = m * p;
    vec3 bg = vec3(1.0, 0.2, 0.8);//(1.0)
    float c = smoothstep(0.01, 0.0, length(q - vec2(0.0, 0.0)) - 0.5);

    vec3 cu = vec3(0.4, 0.7, 0.0);
    q = p;
    q.y = q.y >= 0.0 ? q.y : 1.0;
    float uc = smoothstep(0.01, 0.0, length(q - vec2(0.0, 0.0)) - 0.5);
    q = p;
    float uc2 = smoothstep(0.01, 0.0, length(q - vec2(-0.25, 0.0)) - 0.2475);

    vec3 cd = vec3(1.0, 8.0, 0.0);
    q = p;
    q.y = q.y < 0.0 ? q.y : 1.0;
    float dc = smoothstep(0.01, 0.0, length(q - vec2(0.0, 0.0)) - 0.5);
    q = p;
    float dc2 = smoothstep(0.01, 0.0, length(q - vec2(0.25, 0.0)) - 0.2475);

    vec3 col = mix(bg, vec3(1.0, 1.0, 1.0), c);
    col = mix(col, cu, uc);
    col = mix(col, cd, dc);
    col = mix(col, cu, uc2);
    col = mix(col, cd, dc2);

    gl_FragColor = vec4( col, 1.0 );

}
