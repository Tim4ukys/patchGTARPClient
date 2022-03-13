sampler2D s_2D;

float4 cap_mainPS(float2 tex: TEXCOORD0) : COLOR
{
	float4 color = tex2D(s_2D, tex);
	color += tex2D(s_2D, tex + 0.005);
	color += tex2D(s_2D, tex - 0.005);

	return color / 3;
}

technique technique0
{
	pass p0
	{
		CullMode = None;
		PixelShader = compile ps_2_0 cap_mainPS();
	}
}