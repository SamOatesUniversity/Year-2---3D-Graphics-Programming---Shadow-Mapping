
uniform float4x4 world_view_projection_xform;

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 hposition : POSITION;
	float4 hpos : TEXCOORD;
};

VS_OUTPUT main( VS_INPUT vertex )
{
	VS_OUTPUT output;

	output.hpos = output.hposition = mul( float4( vertex.position, 1.0 ), world_view_projection_xform );
	
	return output;
}

