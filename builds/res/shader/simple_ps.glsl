//uniform sampler2D Texture;

//varying vec2 vCoord;
varying vec4 vColor;

void main()
{
	//vec4 texcrd = texture(Texture, vCoord);
	//gl_FragColor = texcrd * vColor;
	gl_FragColor = vColor.bgra;
}

