#pragma once

#include <cstdint>
#include <memory>

struct MPVector2
{
	float X;
	float Y;
};

struct MPVector3
{
	float X;
	float Y;
	float Z;
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
};

struct MPMatrix4x4
{
	float M[4][4];
};

struct MPString
{
    char* Str;
    int32_t Length;

    MPString(const char* InStr, int32_t InLength)
    {
        Str = new char[InLength];
        std::memcpy(Str, InStr, InLength);
    }

    ~MPString() {
        if (Str) {
            delete[] Str;
        }
    }
};