#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// --- TEXTURAS TERRENO ---
uniform sampler2D textureRock;  // Slot 0
uniform sampler2D textureGrass; // Slot 1

// --- TEXTURA MODELO 3D (NUEVO) ---
uniform sampler2D textureModel; // Slot 2

// --- CONFIGURACIÓN ---
uniform float globalAlpha;      // Para el agua
uniform bool useModelTexture;   // ¿Es un modelo 3D con textura?
uniform bool useSolidColor;     // (Opcional) ¿Es color sólido?
uniform vec3 objectColor;       // Color sólido

void main()
{
    vec4 finalColor;

    if (useModelTexture)
    {
        // 1. MODO MODELO 3D CON TEXTURA (Árbol)
        vec4 texColor = texture(textureModel, TexCoord);
        
        // Si el pixel es transparente (hojas), lo descartamos
        if(texColor.a < 0.1)
            discard;

        // Iluminación básica
        vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.4);

        finalColor = vec4(texColor.rgb * diff, 1.0);
    }
    else if (useSolidColor)
    {
        // 2. MODO COLOR SÓLIDO (Debug o modelos simples)
        vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.4);
        finalColor = vec4(objectColor * diff, 1.0);
    }
    else
    {
        // 3. MODO TERRENO (Mezcla Roca/Césped)
        vec3 norm = normalize(Normal);
        vec3 upDir = vec3(0.0, 1.0, 0.0);
        float slope = dot(norm, upDir);
        
        float blendFactor = smoothstep(0.70, 0.95, slope);
        
        vec4 rockColor = texture(textureRock, TexCoord);
        vec4 grassColor = texture(textureGrass, TexCoord);
        
        finalColor = mix(rockColor, grassColor, blendFactor);
    }

    // Aplicar transparencia global (solo afecta al agua si globalAlpha < 1.0)
    FragColor = vec4(finalColor.rgb, globalAlpha);
}