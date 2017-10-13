void VSPos(
    in float3 iPosition : POSITION,
    in float2 iTexcoord : TEXCOORD,
    in float4 iColor : COLOR,
    out float4 oPosition: SV_POSITION,
    out float2 oTexcoord : TEXCOORD,
    out float4 oColor : COLOR
)
{
    oPosition = float4(iPosition.x, iPosition.y, iPosition.z, 1.0);
    oTexcoord = iTexcoord;
    oColor = iColor;
}

float4 PSYellow(
    in float4 iPosition: SV_POSITION,
    in float2 iTexcoord : TEXCOORD,
    in float4 iColor : COLOR
) : SV_Target
{
    //return float4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow, with Alpha = 1
    return iColor;
}
