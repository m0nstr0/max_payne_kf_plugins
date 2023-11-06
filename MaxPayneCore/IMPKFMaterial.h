#pragma once
#include <cstdint>

#define MPKFMATERIAL_CLASS_ID	Class_ID(0x74450439, 0x62673b6c)

enum class KFMaterialGameVersion : int32_t
{
	kMaxPayne1,
	kMaxPayne2
};

enum class KFDiffuseColorShadingType: int32_t
{
	kNone,
	kColor,
	kGourand
};

enum class KFSpecularColorShadingType : int32_t
{
	kNone,
	kColor,
	kGourand
};

enum class KFDiffuseTextureShadingType : int32_t
{
	kCopy,
	kAdditive,
	kMultiplicative
};

enum class KFReflectionTextureShadingType : int32_t
{
	kCopy,
	kAdditive,
	kMultiplicative
};

enum class KFLitShadingType : int32_t
{
	kPhong,
	kEnvironment,
	kUVW2
};

enum class KFMaskTextureShadingType : int32_t
{
	kUVW1,
	kUVW2
};

struct MPColor
{
	float R;
	float G;
	float B;
	float A;

	MPColor() : R{ 0.f }, G{ 0.f }, B{ 0.f }, A{ 255.f } {}

	MPColor(float InR, float InG, float InB, float InA) : R{ InR }, G{ InG }, B{ InB }, A{ InA } {}
};

class IMPKFMaterial
{
public:
	virtual KFMaterialGameVersion GetGameVersion() = 0;

	virtual bool IsTwoSided() = 0;
	
	virtual bool IsFogging() = 0;

	virtual bool IsInvisibleGeometry() = 0;

	virtual bool HasVertexAlpha() = 0;
	
	virtual float GetVertexAlphaValue() = 0;

	virtual MPColor GetAmbientColor() = 0;

	virtual MPColor GetDiffuseColor() = 0;
	virtual KFDiffuseColorShadingType GetDiffuseColorShadingType() = 0;

	virtual MPColor GetSpecularColor() = 0;
	virtual KFSpecularColorShadingType GetSpecularColorShadingType() = 0;
	virtual float GetSpecularExponent() = 0;

	virtual bool HasDiffuseTexture() = 0;
	virtual class Texmap* GetDiffuseTexture() = 0;
	virtual KFDiffuseTextureShadingType GetDiffuseTextureShadingType() = 0;
	virtual bool HasAlphaCompare() = 0;
	virtual bool HasEdgeBlend() = 0;
	virtual int GetAlphaReferenceValue() = 0;

	virtual bool HasAlphaTexture() = 0;
	virtual class Texmap* GetAlphaTexture() = 0;

	virtual bool HasReflectionTexture() = 0;
	virtual class Texmap* GetReflectionTexture() = 0;
	virtual KFReflectionTextureShadingType GetReflectionTextureShadingType() = 0;
	
	virtual bool HasLit() = 0;
	virtual KFLitShadingType GetLitShadingType() = 0;

	virtual bool HasBumpTexture() = 0;
	virtual class Texmap* GetBumpTexture() = 0;
	virtual float GetBumpEmbossFactor() = 0;
		
	virtual bool HasMaskTexture() = 0;
	virtual class Texmap* GetMaskTexture() = 0;
	virtual KFMaskTextureShadingType GetMaskTextureShadingType() = 0;

	virtual bool IsAllTexturesCorrect() = 0;
};