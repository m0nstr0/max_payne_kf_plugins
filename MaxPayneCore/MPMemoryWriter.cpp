#include "MPMemoryWriter.h"
#include <memory>

void MPMemoryWriter::IncreaseCapacity()
{
	size_t NewCapacity = _Capacity + 4094;
	uint8_t* NewData = new uint8_t[NewCapacity];
	
	std::memset(NewData, 0, NewCapacity);
	std::memcpy(NewData, _Data, _Size);

	delete[] _Data;
	_Data = NewData;

	_Capacity = NewCapacity;
}

void MPMemoryWriter::WriteTag(uint8_t TagID)
{
	Write(&TagID, sizeof(uint8_t));
}

void MPMemoryWriter::Write(const void* Data, size_t Size)
{
	if (Size + _Position >= _Capacity) {
		IncreaseCapacity();
	}

	std::memcpy(&_Data[_Position], Data, Size);
	_Position += Size;

	if (_Position >= _Size) {
		_Size = _Position;
	}
}

void MPMemoryWriter::WriteTagged(uint8_t TagID, const void* Data, size_t Size)
{
	Write(&TagID, sizeof(uint8_t));
	Write(Data, Size);
}

MPMemoryChunkWriter *MPMemoryWriter::CreateChunk(uint8_t TagID, uint32_t ID, uint32_t Version)
{
    return new MPMemoryChunkWriter(TagID, ID, Version);
}

MPMemoryWriter::MPMemoryWriter()
{
	_Position = 0;
	_Size = 0;
	_Capacity = 4096;
	_Data = new uint8_t[_Capacity];

	std::memset(_Data, 0, _Capacity);
}

MPMemoryWriter::~MPMemoryWriter()
{
	if (_Data) {
		delete[] _Data;
		_Data = nullptr;
	}
}

MPMemoryWriter* MPMemoryWriter::operator<<(int16_t Data)
{
	WriteTagged(0x04, &Data, 2);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(uint16_t Data)
{
	WriteTagged(0x05, &Data, 2);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(int32_t Data)
{
	uint32_t Tmp = (uint32_t)abs(Data);

	if (0x7fffff < Tmp) {
		WriteTagged(0x02, &Data, 4);
	}
	else {
		if ((Tmp & 0x7f8000) == 0) {
			if ((Tmp & 0x7f80) == 0) {
				WriteTagged(0x14, &Data, 1);
			}
			else {
				WriteTagged(0x13, &Data, 2);
			}
		}
		else {
			WriteTagged(0x12, &Data, 3);
		}
	}

	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(uint32_t Data)
{
	uint32_t Tmp{Data};

	if (Tmp >> 0x18 == 0) {
		if ((Tmp & 0xff0000) == 0) {
	  		if ((Tmp & 0xff00) == 0) {
				WriteTagged(0x11, &Data, 1);
	 		}
	  		else {
				WriteTagged(0x10, &Data, 2);
	  		}
		} else {
			WriteTagged(0x0F, &Data, 3);
		}		
	} else {
		WriteTagged(0x03, &Data, 4);
	}

	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(int64_t Data)
{
	WriteTagged(0x00, &Data, 8);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(uint64_t Data)
{
	WriteTagged(0x01, &Data, 8);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(char Data)
{
	WriteTagged(0x06, &Data, 1);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(signed char Data)
{
	WriteTagged(0x07, &Data, 1);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(unsigned char Data)
{
	WriteTagged(0x08, &Data, 1);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(float Data)
{
	WriteTagged(0x09, &Data, 4);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(double Data)
{
	WriteTagged(0x0A, &Data, 8);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(bool Data)
{
	WriteTagged(0x0E, &Data, 1);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(const MPVector2& Data)
{
	WriteTag(0x15);
	Write(&Data.X, 4);
	Write(&Data.Y, 4);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(const MPVector3& Data)
{
	WriteTag(0x16);
	Write(&Data.X, 4);
	Write(&Data.Y, 4);
	Write(&Data.Z, 4);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(const MPVector4& Data)
{
	WriteTag(0x17);
	Write(&Data.X, 4);
	Write(&Data.Y, 4);
	Write(&Data.Z, 4);
	Write(&Data.W, 4);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(const MPMatrix2x2& Data)
{
	WriteTag(0x18);
	Write(Data.M, 16);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(const MPMatrix3x3& Data)
{
	WriteTag(0x19);
	Write(Data.M, 36);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(const MPMatrix4x3& Data)
{
	WriteTag(0x1A);
	Write(Data.M, 48);
	return this;
}

MPMemoryWriter* MPMemoryWriter::operator<<(const MPMatrix4x4& Data)
{
	WriteTag(0x1B);
	Write(Data.M, 64);
	return this;
}

MPMemoryWriter *MPMemoryWriter::operator<<(const MPMemoryChunkWriter *Data)
{
	uint8_t TagID = Data->GetTagID();
	uint32_t ID = Data->GetID();
	uint32_t Version = Data->GetVersion();
	uint32_t Size = Data->GetSize() + 13;

	Write(&TagID, 1);
	Write(&ID, 4);
	Write(&Version, 4);
	Write(&Size, 4);
	Write(Data->GetData(), Data->GetSize());

    return this;
}

MPMemoryWriter *MPMemoryWriter::operator<<(const MPString &Data)
{
	WriteTag(0x0D);
	operator<<(Data.Length);
	Write(Data.Str, Data.Length);

    return this;
}

MPMemoryChunkWriter::MPMemoryChunkWriter(uint8_t InTagID, uint32_t InID, uint32_t InVersion):
	MPMemoryWriter(), TagID{ InTagID }, ID{ InID }, Version{ InVersion }
{

}
