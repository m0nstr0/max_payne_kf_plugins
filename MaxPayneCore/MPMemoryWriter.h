#pragma once

#include "MPType.h"
#include <cstdint>

#define MPKFTYPE_MATERIAL_LIST_CHUNK_ID						0x0001000F
#define MPKFTYPE_MESH_CHUNK_ID								0x00010005
#define MPKFTYPE_SKIN_CHUNK_ID								0x00010014
#define MPKFTYPE_KEYFRAME_ANIMATION_CHUNK_ID				0x00010012
#define MPKFTYPE_CAMERA_CHUNK_ID							0x00010001
#define MPKFTYPE_POINT_LIGHT_CHUNK_ID						0x00010002
#define MPKFTYPE_DIRECTIONAL_LIGHT_CHUNK_ID					0x00010003
#define MPKFTYPE_SPOT_LIGHT_CHUNK_ID						0x00010004
#define MPKFTYPE_ENVIRONMENT_CHUNK_ID						0x00010015
#define MPKFTYPE_HELPER_CHUNK_ID							0x00010016
#define MPKFTYPE_POINT_LIGHT_ANIMATION_CHUNK_ID				0x00010017
#define MPKFTYPE_DIRECTIONAL_LIGHT_ANIMATION_CHUNK_ID		0x00010018
#define MPKFTYPE_SPOT_LIGHT_ANIMATION_CHUNK_ID				0x00010019
#define MPKFTYPE_MATERIAL_SUB_CHUNK_ID						0x00010010
#define MPKFTYPE_TEXTURE_SUB_CHUNK_ID						0x00010011
#define MPKFTYPE_NODE_SUB_CHUNK_ID							0x00010000
#define MPKFTYPE_GEOMETRY_SUB_CHUNK_ID						0x00010006
#define MPKFTYPE_POLYGONS_SUB_CHUNK_ID						0x00010007
#define MPKFTYPE_POLYGON_SUB_CHUNK_ID						0x00010008
#define MPKFTYPE_POLYGON_MATERIAL_SUB_CHUNK_ID				0x0001000C
#define MPKFTYPE_UV_MAPPING_SUB_CHUNK_ID					0x0001000e
#define MPKFTYPE_REFERENCE_TO_DATA_SUB_CHUNK_ID				0x0001001A
#define MPKFTYPE_SMOOTHING_SUB_CHUNK_ID						0x0001000B
#define MPKFTYPE_ANIMATION_SUB_CHUNK_ID						0x00010013

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

	MPMemoryChunkWriter* CreateChunk(uint8_t TagID, uint32_t ID, uint32_t Version);

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

	MPMemoryChunkWriter(uint8_t InTagID, uint32_t InID, uint32_t InVersion);

	MPMemoryChunkWriter(const MPMemoryWriter& Other) = delete;

	MPMemoryChunkWriter(MPMemoryWriter&& Other) = delete;

	MPMemoryChunkWriter& operator=(const MPMemoryWriter& Other) = delete;

	MPMemoryChunkWriter& operator=(MPMemoryWriter&& Other) = delete;

	virtual ~MPMemoryChunkWriter() = default;

	uint8_t GetTagID() const { return TagID; }

	uint32_t GetID() const { return ID; }

	uint32_t GetVersion() const { return Version; }

private:
	uint8_t TagID;

	uint32_t ID;

	uint32_t Version;
};