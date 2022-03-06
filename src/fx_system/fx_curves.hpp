#pragma once
#include "fx_structs.hpp"

namespace fx_system
{
	void	FxCurveIterator_FreeRef(FxCurve* releasee);

	float	FX_SampleCurve1D(FxCurve* curve, float scale, float time);
	void	FX_SampleCurve3D(FxCurve* curve, float* value, float scale, float time);

}