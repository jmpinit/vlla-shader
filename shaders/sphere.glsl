// rotating 3d wave field on boundary sphere

#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

void main(void)
{
    float x = gl_FragCoord.x-resolution.x/2.*sin(time/20.);

    float y = gl_FragCoord.y;
    float w = resolution.x;
    float h = resolution.y;
    float pi = 3.14159265;
    float X1 = -10.0;
    float X2 = 10.0;
    float Y1 = -10.0;
    float Y2 = 10.0;
    float t = 1.0*time;

    float X = (((X2-X1)/w)*x+X1)/1.0 *sin(time/20.0);
    float Y = ((((Y2-Y1)/h)*y+Y1)*h/w)/1.0*sin(time/20.0);
    //float R = f1R;
    float R = 4.0;
    float V = R*R-X*X-Y*Y;
    float Z = sqrt(V);
    //float Z = 10.0;
    float freq = 20.0;
    //float dx = sin(t/4.0);
    float phi = t/8.0;
    float Xt = cos(phi)*X-sin(phi)*Z;
    float Zt = sin(phi)*X+cos(phi)*Z;
    float dx = 0.0;
    float f = sin(freq*Xt)+sin(freq*Y)+sin(freq*Zt-t);
    //float f = sin(freq*sqrt((X)*(X)+(Y)*(Y)+(Z)*(Z))-t);
    //f = f + sin(2.0*freq*sqrt((X+dx)*(X+dx)+(Y-0.5)*(Y-0.5)+Z*Z)+t);
    //f = f + sin(freq*sqrt((X)*(X)+Y*Y+(Z-1.0)*(Z-1.0))-t);
    float c = f/3.0;
    if (V<0.0)
    {
        c = 0.0;
        gl_FragColor = vec4(c,0.0,0.0,1.0);
    }
    else
    {
        if (c>0.0)
        {
            gl_FragColor = vec4(c,0.0,0.0,1.0);
        }
        if (c<=0.0)
        {
            gl_FragColor = vec4(abs(c)*abs(sin(time+cos(time)))*2.0,abs(c)*abs(sin(time+sin(time)))*2.0,abs(c)*abs(sin(time))*2.0,1.0);
        }
    }
}
