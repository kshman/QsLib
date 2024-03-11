//uniform sampler2D Texture;
uniform vec4 ConstantPos;
uniform vec4 ConstantColor;
uniform vec4 Diffuse;
uniform vec4 Ambient;
uniform vec4 Specular;

varying vec3 vPosition;
varying vec3 vNormal;
varying vec4 vColor;
//varying vec2 vCoord;

void main()
{
	vec3 lightDir = normalize(vPosition - ConstantPos.xyz);
	float NdotL = max(dot(vNormal, lightDir), 0.0);
	vec4 lightColor = vec4(ConstantColor.xyz * NdotL, 1.0);

	vec4 fragColor = vColor.bgra;
	vec4 color = (fragColor * ((Diffuse + Specular) * lightColor));
	vec4 final = color + (fragColor * Ambient/10.0);

	gl_FragColor = pow(final, vec4(1.0/2.2));
}

