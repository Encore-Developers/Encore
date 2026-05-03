#pragma once 

enum easing_functions: unsigned int
{
	Linear = 0,
	EaseInSine,
	EaseOutSine,
	EaseInOutSine,
	EaseInQuad,
	EaseOutQuad,
	EaseInOutQuad,
	EaseInCubic,
	EaseOutCubic,
	EaseInOutCubic,
	EaseInQuart,
	EaseOutQuart,
	EaseInOutQuart,
	EaseInQuint,
	EaseOutQuint,
	EaseInOutQuint,
	EaseInExpo,
	EaseOutExpo,
	EaseInOutExpo,
	EaseInCirc,
	EaseOutCirc,
	EaseInOutCirc,
	EaseInBack,
	EaseOutBack,
	EaseInOutBack,
	EaseInElastic,
	EaseOutElastic,
	EaseInOutElastic,
	EaseInBounce,
	EaseOutBounce,
	EaseInOutBounce
};

inline const char* getEasingFunctionName(easing_functions function) {
    switch (function) {
    case Linear:
        return "Linear";
    case EaseInSine:
        return "EaseInSine";
    case EaseOutSine:
        return "EaseOutSine";
    case EaseInOutSine:
        return "EaseInOutSine";
    case EaseInQuad:
        return "EaseInQuad";
    case EaseOutQuad:
        return "EaseOutQuad";
    case EaseInOutQuad:
        return "EaseInOutQuad";
    case EaseInCubic:
        return "EaseInCubic";
    case EaseOutCubic:
        return "EaseOutCubic";
    case EaseInOutCubic:
        return "EaseInOutCubic";
    case EaseInQuart:
        return "EaseInQuart";
    case EaseOutQuart:
        return "EaseOutQuart";
    case EaseInOutQuart:
        return "EaseInOutQuart";
    case EaseInQuint:
        return "EaseInQuint";
    case EaseOutQuint:
        return "EaseOutQuint";
    case EaseInOutQuint:
        return "EaseInOutQuint";
    case EaseInExpo:
        return "EaseInExpo";
    case EaseOutExpo:
        return "EaseOutExpo";
    case EaseInOutExpo:
        return "EaseInOutExpo";
    case EaseInCirc:
        return "EaseInCirc";
    case EaseOutCirc:
        return "EaseOutCirc";
    case EaseInOutCirc:
        return "EaseInOutCirc";
    case EaseInBack:
        return "EaseInBack";
    case EaseOutBack:
        return "EaseOutBack";
    case EaseInOutBack:
        return "EaseInOutBack";
    case EaseInElastic:
        return "EaseInElastic";
    case EaseOutElastic:
        return "EaseOutElastic";
    case EaseInOutElastic:
        return "EaseInOutElastic";
    case EaseInBounce:
        return "EaseInBounce";
    case EaseOutBounce:
        return "EaseOutBounce";
    case EaseInOutBounce:
        return "EaseInOutBounce";
    default:
        break;
    }
    return "Invalid";
}

typedef double(*easingFunction)(double);

easingFunction getEasingFunction( easing_functions function );