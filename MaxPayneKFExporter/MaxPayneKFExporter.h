/**
* (c) 2023 Bolotaev Sergey Borisovich aka m0nstr0
* https://github.com/m0nstr0
* https://bolotaev.com
* sergey@bolotaev.com
*/

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <windowsx.h>
#include <max.h>
#include <3dsmaxport.h>
#include <mtl.h>
#include <iparamm2.h>
#include <iparamb2.h>
#include <imtl.h>
#include "resource.h"
#include <IMPKFMaterial.h>
#include <format>
#include <IGame/IGame.h>
#include <IGame/IGameObject.h>
#include <IGame/IGameProperty.h>
#include <IGame/IGameControl.h>
#include <IGame/IGameModifier.h>
#include <IGame/IConversionManager.h>
#include <IGame/IGameError.h>

struct MPKFExporterOptions
{
    int Game{ 0 };
    bool ExportGeometry{ false };
    bool ExportSkinning{ false };
    bool ExportMaterials{ false };
    bool ExportCameras{ false };
    bool ExportLights{ false };
    bool ExportAnimations{ false };
    bool ExportHelpers{ false };
    bool ExportEnvironmets{ false };
    bool CopyTexturesToExportPath{ false };
    bool Scale{ false };
    float ScaleValue{ 0.01f };
    bool RetainHierarchies{ false };
    bool UsePivotAsMeshCenter{ false };
    bool SaveReferencesOnlyOnce{ false };
    bool RemoveFloatingVertices{ false };
    bool RemoveHiddenObjects{ false };
    bool UseGlobalAnimationRange{ false };
    bool LoopWhenFinished{ false };
    bool MaintainMatrixScaling{ false };
    bool LoopInterpolation{ false };
    bool DecEndFrameByOne{ false };
    int SampleRate{ 30 };
    float MinDeltaPosition{ 0.01f };
    float MinDeltaRotation{ 0.01f };
    int StartFrame{ 0 };
    int EndFrame{ 100 };
    int LoopToFrame{ 0 };
    int FrameToFrameRotationInterpolation{ 0 };
    TSTR SkinningPostfix{};
    TSTR SkeletonPostfix{};
    TSTR Paths{};

    void ResetToDefault()
    {
        Game = 0;
        ExportGeometry = false;
        ExportSkinning = false;
        ExportMaterials = false;
        ExportCameras = false;
        ExportLights = false;
        ExportAnimations = false;
        ExportHelpers = false;
        ExportEnvironmets = false;
        CopyTexturesToExportPath = false;
        Scale = false;
        ScaleValue = 0.01f;
        RetainHierarchies = false;
        UsePivotAsMeshCenter = false;
        SaveReferencesOnlyOnce = false;
        RemoveFloatingVertices = false;
        RemoveHiddenObjects = false;
        UseGlobalAnimationRange = false;
        LoopWhenFinished = false;
        MaintainMatrixScaling = false;
        LoopInterpolation = false;
        DecEndFrameByOne = false;
        SampleRate = 30;
        MinDeltaPosition = 0.01f;
        MinDeltaRotation = 0.01f;
        StartFrame = 0;
        EndFrame = 100;
        LoopToFrame = 0;
        FrameToFrameRotationInterpolation = 0;
        Paths = _T("");
        SkinningPostfix = _T("");
        SkeletonPostfix = _T("");
    }
};

extern HINSTANCE hInstance;

extern HWND gMaxWndHandle;

#define MPKFEXPORTER_ERROR_STRING_CONTAINS_NON_ASCII		_T("String \"{}\" contains non-ASCII characters, only english characters are allowed")

#define SHOW_ERROR(Error, ...)  \
    MaxSDK::MaxMessageBox(gMaxWndHandle, (std::format(Error, __VA_ARGS__)).c_str(), _T("Error"), MB_OK | MB_ICONERROR);

#define WRITE_ASCII_STRING(VariableName, MCharString, Writer)									\
    TSTR VariableName = TSTR(MCharString);														\
    if (!IsASCII(VariableName.data())) {														\
        SHOW_ERROR(MPKFEXPORTER_ERROR_STRING_CONTAINS_NON_ASCII, VariableName.data());			\
        return false;																			\
    }																							\
    CStr VariableName##Asc = VariableName.ToCStr();												\
    *Writer << MPString(VariableName##Asc.data(), VariableName##Asc.Length());	

#define IS_ASCII_STRING(MCharString)															\
    if (!IsASCII(MCharString)) {																\
        SHOW_ERROR(MPKFEXPORTER_ERROR_STRING_CONTAINS_NON_ASCII, MCharString);					\
        return false;																			\
    }

TCHAR* GetString(int id);

bool IsASCII(const MCHAR* Str, size_t Length = -1);
