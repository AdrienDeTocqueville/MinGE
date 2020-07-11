in vec4 gl_FragCoord;

out float depth;

void main()
{
	depth = gl_FragCoord.z;
}
