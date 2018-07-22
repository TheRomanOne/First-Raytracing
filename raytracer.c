//Added specular light, shadows and lightSource visualization.
//Special thanks to jackdavenport!


#define MAX_DIST 100.0
#define PI 3.141592654
#define EULER 2.718281828
#define EPSILON 0.001
#define SHADOW 0.01

struct SphereData
{
    float len;
    vec3 center;
    float size;
    vec3 color;
    float specular;
    float dumper;
};
    
SphereData sphere(vec3 center, vec3 ray, float size, vec3 color, float spec, float dump)
{
    SphereData sp;
    
    sp.len = length(ray - center) - size;
    sp.color = color;
    sp.center = center;
    sp.size = size;
    sp.specular = spec;
    sp.dumper = dump;
    
    return sp;
}

SphereData map(vec3 ray)
{
    vec3 p1 = vec3(-5.*sin(iTime/4.), 5.*cos(iTime/4.), 0.);
    vec3 p2 = vec3(4.5*sin(iTime/2. + PI), 0, 4.5*cos(iTime/2. + PI));
    vec3 p3 = vec3(.125 * cos(iTime*3.), .125 * sin(iTime*1.5), .125 * cos(iTime*3.));
    
    SphereData[3] spheres;
    spheres[0] = sphere(p1, ray, 1., vec3(0., 0., .5), 20., 1.5);
    spheres[1] = sphere(p2, ray, .5, vec3(.5, 0., 0.), -1., 8.);
    spheres[2] = sphere(p3, ray, 2.5, vec3(0., .5, 0.), 15., 2.5);
    
    SphereData temp = spheres[0];
    
    for(int index = 0; index < 3; index++)
        if(spheres[index].len < temp.len)
            temp = spheres[index];   
        
    return temp;
}

vec3 getNormal(vec3 ray)
{
	vec3 grad = vec3
    (
      map(ray + vec3(EPSILON, 0, 0)).len - map(ray - vec3(EPSILON, 0, 0)).len,
      map(ray + vec3(0, EPSILON, 0)).len - map(ray - vec3(0, EPSILON, 0)).len,
      map(ray + vec3(0, 0, EPSILON)).len - map(ray - vec3(0, 0, EPSILON)).len
    );
    
    return normalize(grad);
}

float trace(vec3 origin, vec3 dir)
{
 	float dist = 0.;
    for(int i = 0;i<256;i++)
        if(dist < MAX_DIST)
        {
        	vec3 ray = origin + dir * dist;
        	dist += map(ray).len;
        }else return -1.;
            
    return dist;
}

mat4 viewMatrix(vec3 eye, vec3 center, vec3 up)
{
    vec3 f = normalize(center - eye);
    vec3 s = cross(f, up);
    vec3 u = cross(s, f);
    return mat4(
        vec4(s, 0.0),
        vec4(u, 0.0),
        vec4(-f, 0.0),
        vec4(0.0, 0.0, 0.0, 1.)
    );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = (fragCoord - iResolution.xy*0.5)/iResolution.y;
        
    vec3 ray = normalize(vec3(uv, 1.));
    vec3 origin = vec3(0./*5.*sin(iTime/3.)*/,0., -6.);
    vec3 camDir = normalize(-origin);
    
    //vec3 light = vec3(20.*sin(iTime/3.), 0., -20.*cos(iTime/3.));
    vec3 light = vec3(20.*sin(iTime/5.), 50. * cos(iTime * 2.) * sin(iTime/3.), 20.*cos(iTime/5.));
    vec3 lightDir = normalize(light);
    vec3 lightColor = vec3(0.69, 0.0015, .7);
    
    ray = (viewMatrix(ray, vec3(0.), vec3(.0, 1.0, 0.0)) * vec4(ray, 0.)).xyz;

    float dist = trace(origin, ray);
    vec3 color = vec3(0.);
    if(dist > 0.)
    {
        vec3 position = origin + ray * dist;
        SphereData sp = map(position);
        vec3 baseColor = sp.color;

        vec3 normal = getNormal(position);
         
        float diffuse = max(SHADOW, dot(normal, lightDir));
        float specular = 0.;
        if(sp.specular > 0. )
            specular = pow(max(0., dot(normalize(reflect(ray, normal)), lightDir)), sp.specular)/(sp.dumper);
        
        
        if(trace(position + lightDir * 0.1, lightDir) > 0.)
        {
            specular = 0.;
            diffuse = SHADOW;
        }
        
        float shineBack = 0.;//pow(max(0., dot(lightDir, ray)), 60.);
        
        color = baseColor * diffuse + (shineBack*0.1 + specular) * sqrt(lightColor);
        
    } else
    {
     	float shine = pow(max(0., dot(lightDir, ray)), 20.);
        float shineBack = pow(max(0., dot(lightDir, camDir)), 2.2);
        color = (shineBack + shine) * lightColor;
    }
    
    fragColor = vec4(sqrt(color), 1.);
}
