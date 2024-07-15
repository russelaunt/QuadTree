cbuffer BasicVertexConstantData : register(b0)
{
    matrix model;
};

struct VSInput
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;

    float4 pos = float4(vsInput.position, 1.0);
    vsOutput.position = mul(pos, model);
    
    //vsOutput.position = pos;
    vsOutput.color = float4(vsInput.color, 1.0);
    
    return vsOutput;
}