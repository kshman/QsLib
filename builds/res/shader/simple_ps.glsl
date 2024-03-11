//uniform sampler2D Texture;

uniform vec4 ConstantDir;
uniform vec4 ConstantColor;
uniform vec4 Diffuse;
uniform vec4 Ambient;
uniform vec4 Specular;
uniform vec4 ViewPos;

varying vec3 vPosition;
varying vec3 vNormal;
varying vec4 vColor;
//varying vec2 vCoord;

void main()
{
	vec3 norm = normalize(vNormal);
	vec3 lightDir = normalize(-ConstantDir.xyz);
	float ndotl = max(dot(norm, lightDir), 0.0);
	vec4 lightColor = vec4(ConstantColor.rgb * ndotl, 1.0);
	vec4 color = vColor.bgra;
	vec4 ambient = color * ((Ambient / 10.0) * 4.0);
	color = color * (Diffuse + Specular) * lightColor + ambient;
	gl_FragColor = pow(color, vec4(1.0/2.2));
}

