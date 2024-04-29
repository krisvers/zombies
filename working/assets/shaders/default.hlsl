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

cbuffer ubo : register(b0)
{
	float4x4 model;
	float4x4 view;
	float4x4 projection;
};

vertex_output vsmain(vertex_input input)
{
	float4x4 mvp = view;
	
	vertex_output output;
	output.position = float4(input.position, 1.0f);
	output.normal = input.normal;
	output.color = (mul(float4(input.position, 1.0f), mvp).xyz + 1) / 2 + 1;
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