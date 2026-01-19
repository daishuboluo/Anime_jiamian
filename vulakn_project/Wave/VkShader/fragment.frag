#version 450

layout(location = 0) in float vLineCoord;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 silver = vec3(0.75, 0.75, 0.80);
    vec3 white  = vec3(1.0, 1.0, 1.0);
    
    // 簡單的噪點式光澤（基於螢幕座標）
    float shine = sin(gl_FragCoord.x * 0.1 + gl_FragCoord.y * 0.1) * 0.5 + 0.5;
    
    vec3 finalColor = mix(silver, white, shine);
    
    outColor = vec4(finalColor, 1.0);
}