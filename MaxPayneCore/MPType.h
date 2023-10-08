#pragma once

#include <cstdint>
#include <memory>

struct MPVector2
{
	float X;
	float Y;

	MPVector2(float InX, float InY) : X{ InX }, Y{ InY } {}
};

struct MPVector3
{
	float X;
	float Y;
	float Z;

	MPVector3(float InX, float InY, float InZ) : X{ InX }, Y{ InY }, Z{ InZ } {}

	MPVector3(const MPVector3& Other) 
	{
		X = Other.X;
		Y = Other.Y;
		Z = Other.Z;
	}

	MPVector3& operator=(const MPVector3& Other)
	{
		X = Other.X;
		Y = Other.Y;
		Z = Other.Z;

		return *this;
	}

	MPVector3 ScaleBy(float Value) const 
	{
		return MPVector3(X * Value, Y * Value, Z * Value);
	}
};

struct MPVector4
{
	float X;
	float Y;
	float Z;
	float W;
};

struct MPMatrix2x2
{
	float M[2][2];
};

struct MPMatrix3x3
{
	float M[3][3];
};

struct MPMatrix4x3
{
	float M[4][3];

	MPMatrix4x3(float M11, float M12, float M13, float M21, float M22, float M23, float M31, float M32, float M33, float M41, float M42, float M43)
	{
		M[0][0] = M11;
		M[0][1] = M12;
		M[0][2] = M13;
		M[1][0] = M21;
		M[1][1] = M22;
		M[1][2] = M23;
		M[2][0] = M31;
		M[2][1] = M32;
		M[2][2] = M33;
		M[3][0] = M41;
		M[3][1] = M42;
		M[3][2] = M43;
	}
};

struct MPMatrix4x4
{
	float M[4][4];
};

struct MPString
{
    const char* Str;
    int32_t Length;

    MPString(const char* InStr, int32_t InLength): Str{InStr}, Length{InLength}
    {
    }
};