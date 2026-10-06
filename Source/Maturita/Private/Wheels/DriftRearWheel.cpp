
#include "Wheels/DriftRearWheel.h"

UDriftRearWheel::UDriftRearWheel()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
	MaxSteerAngle = 0.0f;
}

