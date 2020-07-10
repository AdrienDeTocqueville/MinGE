in vec4 gl_FragCoord;

out vec4 depth;

void main()
{
	depth = vec4(gl_FragCoord.z, 0.0, 0.0, 0.0);
}
