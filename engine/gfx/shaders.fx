void VSPos(
    in float3 iPosition : POSITION,
    in float2 iTexcoord : TEXCOORD,
    in float4 iColor : COLOR,
    out float4 oPosition : SV_POSITION,
    out float2 oTexcoord : TEXCOORD,
    out float4 oColor : COLOR
    )
{
    oPosition = float4(iPosition.x, iPosition.y, iPosition.z, 1.0);
    oTexcoord = iTexcoord;
    oColor = iColor;
}

Texture2D texture0;

SamplerState sampleNearest
{
    Filter = POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

float4 PSTextureColor(
    in float4 iPosition : SV_POSITION,
    in float2 iTexcoord : TEXCOORD,
    in float4 iColor : COLOR
    ) : SV_Target
{
    return texture0.Sample(sampleNearest, iTexcoord) * iColor;
}

float4 PSColor(
    in float4 iPosition : SV_POSITION,
    in float2 iTexcoord : TEXCOORD,
    in float4 iColor : COLOR
    ) : SV_Target
{
    return iColor;
}
