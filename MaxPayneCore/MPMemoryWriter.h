#pragma once

#include "MPType.h"
#include <cstdint>

class MPMemoryChunkWriter;

class MPMemoryWriter
{
public:
	MPMemoryWriter();

	MPMemoryWriter(const MPMemoryWriter& Other) = delete;

	MPMemoryWriter(MPMemoryWriter&& Other) = delete;

	MPMemoryWriter& operator=(const MPMemoryWriter& Other) = delete;

	MPMemoryWriter& operator=(MPMemoryWriter&& Other) = delete;

	virtual ~MPMemoryWriter();

	MPMemoryWriter* operator<<(int16_t Data);

	MPMemoryWriter* operator<<(uint16_t Data);

	MPMemoryWriter* operator<<(int32_t Data);

	MPMemoryWriter* operator<<(uint32_t Data);

	MPMemoryWriter* operator<<(int64_t Data);

	MPMemoryWriter* operator<<(uint64_t Data);

	MPMemoryWriter* operator<<(char Data);

	MPMemoryWriter* operator<<(signed char Data);

	MPMemoryWriter* operator<<(unsigned char Data);

	MPMemoryWriter* operator<<(float Data);

	MPMemoryWriter* operator<<(double Data);

	MPMemoryWriter* operator<<(bool Data);

	MPMemoryWriter* operator<<(const MPVector2& Data);

	MPMemoryWriter* operator<<(const MPVector3& Data);

	MPMemoryWriter* operator<<(const MPVector4& Data);

	MPMemoryWriter* operator<<(const MPMatrix2x2& Data);

	MPMemoryWriter* operator<<(const MPMatrix3x3& Data);

	MPMemoryWriter* operator<<(const MPMatrix4x3& Data);

	MPMemoryWriter* operator<<(const MPMatrix4x4& Data);

	MPMemoryWriter* operator<<(const MPMemoryChunkWriter* Data);

	MPMemoryWriter* operator<<(const MPString& Data);

	size_t GetSize() const { return _Size; }
	
	uint8_t* GetData() const { return _Data; }

	void WriteTag(uint8_t TagID);

	void Write(const void* Data, size_t Size);

	void WriteTagged(uint8_t TagID, const void* Data, size_t Size);

	MPMemoryChunkWriter* CreateChunk(uint8_t TagID, uint32_t ID, uint8_t Version);

protected:
	void IncreaseCapacity();

	size_t _Position;

	uint8_t* _Data;

	size_t _Capacity;

	size_t _Size;
};


class MPMemoryChunkWriter : public MPMemoryWriter
{
public:
	MPMemoryChunkWriter() = delete;

	MPMemoryChunkWriter(uint8_t InTagID, uint32_t InID, uint8_t InVersion);

	MPMemoryChunkWriter(const MPMemoryWriter& Other) = delete;

	MPMemoryChunkWriter(MPMemoryWriter&& Other) = delete;

	MPMemoryChunkWriter& operator=(const MPMemoryWriter& Other) = delete;

	MPMemoryChunkWriter& operator=(MPMemoryWriter&& Other) = delete;

	virtual ~MPMemoryChunkWriter() = default;

	uint8_t GetTagID() const { return TagID; }

	uint8_t GetID() const { return ID; }

	uint8_t GetVersion() const { return Version; }

private:
	uint8_t TagID;

	uint32_t ID;

	uint8_t Version;
};