#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// --- TEXTURAS ---
uniform sampler2D textureRock;  // Slot 0
uniform sampler2D textureGrass; // Slot 1 (Usado también para AGUA en App.cpp)
uniform sampler2D textureModel; // Slot 2

// --- CONFIGURACIÓN ---
uniform float globalAlpha;
uniform bool useModelTexture;
uniform bool useSolidColor;
uniform vec3 objectColor;

// --- NUEVO: Animación ---
uniform float uTime;
uniform bool isWater;

void main()
{
    vec4 finalColor;

    // 1. MODO AGUA (NUEVO: Animación de flujo)
    if (isWater)
    {
        // Movemos las coordenadas de textura con el tiempo
        vec2 movingTexCoord = TexCoord + vec2(uTime * 0.05, uTime * 0.05);
        
        // Usamos 'textureGrass' porque en App.cpp pasamos la textura de agua al slot 1
        vec4 waterTex = texture(textureGrass, movingTexCoord);
        
        // Un toque azulado para que se vea más bonito
        vec4 blueTint = vec4(0.0, 0.4, 0.8, 1.0);
        finalColor = mix(waterTex, blueTint, 0.3);
    }
    // 2. MODO ÁRBOL
    else if (useModelTexture)
    {
        vec4 texColor = texture(textureModel, TexCoord);
        
        if(texColor.a < 0.1)
            discard;

        vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.4);

        finalColor = vec4(texColor.rgb * diff, 1.0);
    }
    // 3. MODO CASTILLO (Color sólido)
    else if (useSolidColor)
    {
        vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.4);
        finalColor = vec4(objectColor * diff, 1.0);
    }
    // 4. MODO TERRENO (Mezcla normal)
    else
    {
        vec3 norm = normalize(Normal);
        vec3 upDir = vec3(0.0, 1.0, 0.0);
        float slope = dot(norm, upDir);
        
        float blendFactor = smoothstep(0.70, 0.95, slope);
        
        vec4 rockColor = texture(textureRock, TexCoord);
        vec4 grassColor = texture(textureGrass, TexCoord);
        
        finalColor = mix(rockColor, grassColor, blendFactor);
    }

    // Aplicar transparencia global
    FragColor = vec4(finalColor.rgb, globalAlpha);
}