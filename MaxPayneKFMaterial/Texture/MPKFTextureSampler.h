/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#pragma once

#include "MaxPayneKFMaterial.h"
#include <max.h>

class MPKFTexture;

class MPKFTextureSampler : public MapSampler
{
public:
	MPKFTextureSampler();

	MPKFTextureSampler(MPKFTexture* c);

	virtual ~MPKFTextureSampler() = default;

	void Set(MPKFTexture* c);

	AColor Sample(ShadeContext& sc, float u, float v);

	AColor SampleFilter(ShadeContext& sc, float u, float v, float du, float dv);

private:
	MPKFTexture* Texture;
};