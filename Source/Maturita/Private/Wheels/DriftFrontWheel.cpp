
#include "Wheels/DriftFrontWheel.h"

UDriftFrontWheel::UDriftFrontWheel()
{
	AxleType = EAxleType::Front;
	bAffectedByHandbrake = false;
	bAffectedByEngine = false;
	MaxSteerAngle = 40.0f;
}

