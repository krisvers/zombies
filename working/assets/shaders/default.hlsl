struct vertex_input
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float3 color : COLOR0;
	float2 texcoord : TEXCOORD0;
};

struct vertex_output
{
	float4 position : SV_Position;
	float3 normal : NORMAL0;
	float3 color : COLOR0;
	float2 texcoord : TEXCOORD0;
};

vertex_output vsmain(vertex_input input)
{
	vertex_output output;
	output.position = float4(input.position, 1.0f);
	output.normal = input.normal;
	output.color = input.color;
	output.texcoord = input.texcoord;
	return output;
}

struct pixel_output
{
	float4 color : SV_Target;
};

pixel_output psmain(vertex_output input)
{
	pixel_output output;
	output.color = float4(input.color, 1.0f);
	return output;
}