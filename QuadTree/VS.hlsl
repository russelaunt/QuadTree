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

    vsOutput.position = float4(vsInput.position, 1.0);
    vsOutput.color = float4(vsInput.color, 1.0);

    return vsOutput;
}