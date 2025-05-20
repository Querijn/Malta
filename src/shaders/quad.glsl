
@vs vs
in vec2 position;
out vec2 uv;
out vec2 imageDims;

layout(binding=0) uniform VertParams
{
	vec2 screenSize;
	vec2 imageSize;
};

void main()
{
	float screenAspect = screenSize.x / screenSize.y;
	float imageAspect = imageSize.x / imageSize.y;

	vec2 scale = vec2(1.0);
	if (screenAspect > imageAspect)
		scale.y = screenAspect / imageAspect;
	else
		scale.x = imageAspect / screenAspect;

	vec2 pos = position * scale;
	uv = position * 0.5 + 0.5;
	uv.y = 1.0 - uv.y;
	imageDims = imageSize;
	gl_Position = vec4(pos, 0.0, 1.0);
}
@end

@fs fs
in vec2 uv;
in vec2 imageDims;
out vec4 fragColor;

layout(binding=1) uniform FragParams
{
	float dayCycleTime;
	float waterTime;
	float windFrequency;
};

layout(binding=0) uniform sampler smp;

layout(binding=0) uniform texture2D dayBackTex;
layout(binding=1) uniform texture2D dayFrontTex;

layout(binding=2) uniform texture2D eveBackTex;
layout(binding=3) uniform texture2D eveFrontTex;

layout(binding=4) uniform texture2D nightBackTex;
layout(binding=5) uniform texture2D nightFrontTex;

layout(binding=6) uniform texture2D waterMask;
layout(binding=7) uniform texture2D windMap;
layout(binding=8) uniform texture2D starMap;

#define M_PI 3.1415926535897932384626433832795
float easeOutQuint(float x) { return 1.0 - pow(1.0 - x, 5.0); }

vec4 calculateStarColor(vec4 backColor, vec2 backUv)
{
	vec2 mid = vec2(0.5, 1);
	vec2 delta = normalize(backUv - mid);
	vec2 rotAmount = vec2(-delta.y, delta.x) * 0.1;

	float dayPhase = dayCycleTime * 3.0;
	float beginOfNight = 1.0;
	float heightOfNight = 2.0;
	float endOfNight = 3.0;

	float starShininess = 1.0 - abs(heightOfNight - dayPhase);
	float cycleProgress = (dayPhase - beginOfNight) / (endOfNight - beginOfNight);

	vec2 starUv = mix(backUv + rotAmount, backUv, cycleProgress);
	vec4 starMapColor = texture(sampler2D(starMap, smp), starUv);
	
	starMapColor.a *= starShininess * starShininess;
	return mix(backColor, backColor + starMapColor, starMapColor.a);
}

void main()
{
	vec2 backUv = uv;
	vec2 frontUv = uv;
	vec4 waterMaskColor  = texture(sampler2D(waterMask, smp), uv);
	vec2 imageStep = 1.0 / imageDims;

	// Water wavey effect
	if (waterMaskColor.r > 0.99)
	{
		backUv.x += 0.005 * uv.x * sin(waterTime * M_PI * 2.0);
		backUv.y += 0.02 * uv.y * sin(waterTime * M_PI);
	}

	vec2 windMapVector = texture(sampler2D(windMap, smp), uv).xy;
	if (windMapVector.r > 0.99)
	{
		vec2 mid = vec2(0.5, 0.5);
		vec2 delta = normalize(frontUv - mid);
		vec2 sway = vec2(-delta.y, delta.x);

		float invLength = 1.0 - length(frontUv - mid);

		float t = sin(frontUv.y * M_PI * 2.0 + waterTime * windFrequency * M_PI * 2.0);
		frontUv += sway * imageStep * invLength * 2.0 * t;
	}

	vec4 dayBackColor    = texture(sampler2D(dayBackTex, smp),    backUv);
	vec4 eveBackColor    = texture(sampler2D(eveBackTex, smp),    backUv);
	vec4 nightBackColor  = texture(sampler2D(nightBackTex, smp),  backUv);
	vec4 dayFrontColor   = texture(sampler2D(dayFrontTex, smp),   frontUv);
	vec4 eveFrontColor   = texture(sampler2D(eveFrontTex, smp),   frontUv);
	vec4 nightFrontColor = texture(sampler2D(nightFrontTex, smp), frontUv);

	// Day/Night cycle
	vec4 backColor;
	vec4 frontColor;
	if (dayCycleTime < 1.0 / 3.0)
	{
		float t = dayCycleTime * 3.0;
		backColor = mix(dayBackColor, eveBackColor, t);
		frontColor = mix(dayFrontColor, eveFrontColor, t);
	}
	else if (dayCycleTime < 2.0 / 3.0)
	{
		float t = dayCycleTime * 3.0 - 1.0;

		backColor = mix(eveBackColor, nightBackColor, t);
		backColor = calculateStarColor(backColor, backUv);

		frontColor = mix(eveFrontColor, nightFrontColor, t);
	}
	else
	{
		float t = dayCycleTime * 3.0 - 2.0;

		backColor = mix(nightBackColor, dayBackColor, t);
		backColor = calculateStarColor(backColor, backUv);

		frontColor = mix(nightFrontColor, dayFrontColor, t);

	}

	if (waterMaskColor.r > 0.99)
	{
		backColor += 0.1 * backColor * sin(waterTime * M_PI * 10.0);
	}

	// Edge fade
	float edge = 0.1;
	float edgeFade = easeOutQuint(smoothstep(0.0, edge, uv.x)) *
					 easeOutQuint(smoothstep(1.0, 1.0 - edge, uv.x)) *
					 easeOutQuint(smoothstep(1.0, 1.0 - edge, uv.y));

	fragColor = mix(backColor, frontColor, frontColor.a) * edgeFade;
}
@end

@program quad vs fs