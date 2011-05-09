
uniform float4x4 world_xform;
uniform float4x4 world_view_projection_xform;

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VS_OUTPUT
{
	float4 hposition : POSITION;
	float3 world_position : TEXCOORD0;
	float3 world_normal : TEXCOORD1;
};

VS_OUTPUT main( VS_INPUT vertex )
{
	VS_OUTPUT output;

	//Standerd transformations
	output.hposition = mul( float4( vertex.position, 1.0 ), world_view_projection_xform );
	output.world_position = mul( float4( vertex.position, 1.0 ), world_xform ).xyz;
	output.world_normal = mul( float4( vertex.normal, 0.0 ), world_xform ).xyz;
	
	return output;
}
