/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#include "MPKFTextureSampler.h"
#include "MPKFTexture.h"

inline MPKFTextureSampler::MPKFTextureSampler()
	: Texture{ nullptr }
{}

MPKFTextureSampler::MPKFTextureSampler(MPKFTexture* Other)
	: Texture{ Other }
{}

void MPKFTextureSampler::Set(MPKFTexture* Other)
{
	Texture = Other;
}

AColor MPKFTextureSampler::Sample(ShadeContext& sc, float u, float v)
{
	BMM_Color_fl c;

	int x, y;
	float fu, fv;

	fu = frac(u);
	fv = 1.0f - frac(v);

	if (fu >= 1.0f)
		fu = 0.0f;
	if (fv >= 1.0f)
		fv = 0.0f;

	if (Bitmap* bm = Texture->GetBitmap(); bm) {
		x = (int)(fu * bm->Width());
		y = (int)(fv * bm->Height());
		bm->GetLinearPixels(x, y, 1, &c);
	}

	//switch (alphaSource) {
	//case ALPHA_NONE:  c.a = 1.0f; break;
	//case ALPHA_RGB:   c.a = (c.r + c.g + c.b) / 3;
		//if (c.a > 1)c.a = 1;
		//else if (c.a < 0)c.a = 0;
		//break;
		//  TBD
		// XPCOL needs to be handled in bitmap for filtering. 
		// Need to open a bitmap with this property.
		//	case ALPHA_XPCOL:  break; 
	//}

	return c;
}

AColor MPKFTextureSampler::SampleFilter(ShadeContext& sc, float u, float v, float du, float dv)
{
	BMM_Color_fl c;
	float fu, fv;
	fu = frac(u);
	fv = 1.0f - frac(v);

	if (Bitmap* bm = Texture->GetBitmap(); bm) {
		bm->GetFiltered(fu, fv, du, dv, &c);
	}

	/*switch (alphaSource) {
	case ALPHA_NONE:  c.a = 1.0f; break;
	case ALPHA_RGB:   c.a = (c.r + c.g + c.b) / 3;
		if (c.a > 1)c.a = 1;
		else if (c.a < 0)c.a = 0;
		break;
	}*/

	return c;
}
