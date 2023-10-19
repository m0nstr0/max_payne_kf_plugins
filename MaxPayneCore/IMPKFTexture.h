#pragma once
#include <cstdint>

#define MPKFTEXTURE_CLASS_ID	Class_ID(0x35bb1f36, 0x2a4a7430)

enum class KFTextureFiltering : int32_t
{
	kNone,
	kBilliner,
	kAuto,
	kTrillinear,
	kAnisotropic
};

enum class KFTextureAnimationEndCondition : int32_t
{
	kLoop,
	kPingPong,
	kHold
};

class IMPKFTexture
{
public:
	virtual const MCHAR* GetTextureName() = 0;
	virtual int32_t GetTexturesCount() = 0;
	virtual const TSTR GetTextureFileName(int32_t TextureIndex) = 0;
	virtual int32_t GetMipMapsNum() = 0;
	virtual bool IsMipMapsAuto() = 0;
	virtual KFTextureFiltering GetFiltering() = 0;
	virtual bool IsAnimationAutomaticStart() = 0;
	virtual bool IsAnimationRandomStartFrame() = 0;
	virtual int32_t GetAnimationStartFrame() = 0;
	virtual int32_t GetAnimationFPS() = 0;
	virtual KFTextureAnimationEndCondition GetAnimationEndCondition() = 0;
};