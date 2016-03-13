
struct VS_INPUT
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 tex : TEXCOORD0;
	row_major float4x4 worldMatrix : MATRIX;
	float4 diffuseColor : COLOR;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float3 worldPosition : TEXCOORD0;
	float4 color : TEXCOORD1;
	float2 tex : TEXCOORD2;
	float3 centerPos : TEXCOORD3;
};

//-------------------------------------------------------------

struct Light
{
	float3 position;
	uint type;
	float4 diffuseColor;
	float4 attenuation;
};

cbuffer vscbMesh0 : register( b0 )
{
	row_major float4x4 g_viewProjectionMatrix;

	float4 g_ambientColor;

	Light g_lights[4];
}

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	const float4 posWS = mul(input.pos, input.worldMatrix);

	output.worldPosition = posWS.xyz;
	output.centerPos = input.diffuseColor.yzw;

	output.pos = mul(posWS,g_viewProjectionMatrix);

	output.tex.x = input.diffuseColor.x + input.tex.x * 1.0/3.0;
	output.tex.y = input.tex.y;

	const float brightness = 1.0 - saturate((180 - output.worldPosition.y) / 30.0) * 1;

	output.color = float4(brightness, brightness, brightness, output.worldPosition.z < 950 ? 1.0 : (1000.0 - output.worldPosition.z)/50.0);

	return output;
}

//-------------------------------------------------------------

Texture2D texture0 : register( t0 );
SamplerState sampler0 : register( s0 );

cbuffer pscbMesh0 : register( b0 )
{
	float3 g_cameraPosition;
	uint g_fogType;
	float4 g_fogParam;
	float4 g_fogColor;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	const float3 worldPos = input.worldPosition;
	const float3 sunPos = float3(10000, 14000, -2000);
	const float3 centerPos = input.centerPos;
	const float b = saturate(dot(normalize(sunPos - worldPos), normalize(worldPos - centerPos))) * 0.5 + 0.5;

	float4 color = texture0.Sample(sampler0,input.tex) * input.color;
	color.rgb *= b;

	return color;
}
