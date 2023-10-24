#include "MPKFExporter.h"

#include <IGame/IGame.h>
#include <IGame/IGameObject.h>
#include <IGame/IGameProperty.h>
#include <IGame/IGameControl.h>
#include <IGame/IGameModifier.h>
#include <IGame/IConversionManager.h>
#include <IGame/IGameError.h>
#include <3dsmaxport.h>
#include <MPMemoryWriter.h>
#include <IMPKFTexture.h>
#include <set>
#include <vector>
#include <map>
#include <format>
#include <string>
#include <CS/Phyexp.h>

#define MPKFEXPORTER_ERROR_UNSUPORTED_MATERIAL				_T("Material \"{}\" has unsupported class \"{}\", only KF Materials and Multi/Sub-Object are allowed")
#define MPKFEXPORTER_ERROR_UNSUPORTED_TEXTURE				_T("Texture \"{}\" has unsupported class \"{}\", only KF Textures are allowed")
#define MPKFEXPORTER_ERROR_STRING_CONTAINS_NON_ASCII		_T("String \"{}\" contains non-ASCII characters, only english characters are allowed")
#define	MPKFEXPORTER_ERROR_TEXTURE_EMPTY					_T("There is no texture in the slot \"{}\" in the material \"{}\"")
#define	MPKFEXPORTER_ERROR_TEXTURE_EMPTY_BITMAP				_T("There is no bitmap in the texture \"{}\"")
#define	MPKFEXPORTER_ERROR_MATERIAL_WRONG_TARGET			_T("Export target is \"{}\" but the material's \"{}\" target is \"{}\"")
#define	MPKFEXPORTER_ERROR_MESH_TRIANGULATE					_T("Unable to triangulate mesh \"{}\"")
#define MPKFEXPORTER_ERROR_TEXTURE_START_FRAME				_T("Texture \"{}\" has only \"{}\" frames but the field Start Frame is set to \"{}\"")
#define MPKFEXPORTER_ERROR_UNSUPPORTED_OBJECTS				_T("The scene has an unsupported object \"{}\"")
#define MPKFEXPORTER_ERROR_OBJECTS_WITHOUT_MATERIAL			_T("Object \"{}\" doesn't have any material")
#define MPKFEXPORTER_ERROR_MP2_SKIN_BONES_NUM				_T("Max Payne 2: Object \"{}\" has more that 4 bones per vertex")
#define MPKFEXPORTER_ERROR_MP1_SKIN_BONES_NUM				_T("Max Payne 1: Object \"{}\" has more that 3 bones per vertex")
#define MPKFEXPORTER_ERROR_SKIN_FREE_VERTEX					_T("There are some vertices have zero weight sum \"{}\"")
#define MPKFEXPORTER_ERROR_SKIN_NULL_BONE					_T("Bone with index \"{}\" not found")
#define MPKFEXPORTER_ERROR_SKIN_NULL_BONES					_T("There are no bones")
#define MPKFEXPORTER_ERROR_SKIN_MORE_THAN_ONE_ROOT			_T("There are more than one root bone")
#define MPKFEXPORTER_ERROR_SKIN_NOT_FOUND					_T("Skin or Physique modifier not found")
#define MPKFEXPORTER_ERROR_SKIN_WRONG_VERTEX_TYPE			_T("Unknown vertex type. Only RIGID and RIGI_BLENDED are supported")

#define MPKFEXPORTER_ERROR_HIERARCHY_HIDDEN_HAS_CHILD		_T("Texture \"{}\" has only \"{}\" frames but the field Start Frame is set to \"{}\"")
#define MPKFEXPORTER_ERROR_HIERARCHY_HIDDEN_HAS_CHILD		_T("Texture \"{}\" has only \"{}\" frames but the field Start Frame is set to \"{}\"")
#define MPKFEXPORTER_ERROR_UNSUPPORTED_OBJECT_HAS_CHILD		_T("Texture \"{}\" has only \"{}\" frames but the field Start Frame is set to \"{}\"")

#define SHOW_ERROR(Error, ...)																							\
	MaxSDK::MaxMessageBox(GetMaxHWND(), (std::format(Error, __VA_ARGS__)).c_str(), _T("Error"), MB_OK | MB_ICONERROR);

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

bool IsASCII(const MCHAR* Str, size_t Length = -1)
{
	size_t StrIndex = 0;
	while (Str && Str[StrIndex] != '\0') {
		if (Length > 0 && StrIndex >= Length) {
			return true;
		}
		if (isascii(Str[StrIndex]) == 0) {
			return false;
		}
		StrIndex++;
	}
	return true;
}

struct MPKFAnimationFrameExporterContext
{
	uint32_t FrameID;
	GMatrix Transform;
};

struct MPKFAnimationExporterContext 
{
	TSTR ObjectName;
	TSTR ParentName;
	int32_t SampleRate;
	bool LoopWhenFinished;
	bool LoopInterpolation;
	uint32_t NumTotalKeyFrames;
	std::vector<MPKFAnimationFrameExporterContext> Frames;
	uint32_t LoopToFrame;
	int32_t FrameToFrameRotationInterpolation;
	bool MaintainMatrixScaling;
};

struct MPKFMeshExporterContext
{
	std::vector<MPVector3> MeshVertices;
	std::vector<int> MeshMaxVertexIndex;
	std::vector<MPVector2> MeshTexVertices;
	std::vector<MPVector3> MeshNormals;
	std::vector<uint16_t> MeshIndices;
	std::vector<uint32_t> MeshVerticesPerPrimitive;
	std::vector<uint32_t> MeshIndicesPerPrimitive;
	std::vector<IGameMaterial*> MaterialPerPrimitive;
	TSTR SkinObjectName;
	bool HasSkin;
	std::vector<IGameNode*> SkinBones;
	std::vector<int> SkinBonesNumPerVertex;
	std::vector<float> SkinWeightsPerVertex;
	std::vector<int> SkinBonesIDsPerVertex;
	std::vector<int> SkinVertexOffset;
	std::vector<MPKFAnimationExporterContext> Animations;
};

struct MPKFNodeInfo
{
	IGameNode* Node;
	IGameNode* ParentNode;
	bool Export;
	IGameObject::ObjectTypes Type;
};

struct MPKFGlobalExporterContext
{
	MPMemoryWriter TargetMemoryWriter;
	MPMemoryWriter SkinMemoryWriter;
	MPMemoryWriter OtherMemoryWriter;
	MPMemoryWriter AnimationMemoryWriter;
	MPMemoryWriter MaterialMemoryWriter;
	std::vector<IGameMaterial*> Materials;
	std::vector<MPKFNodeInfo> Nodes;
	bool ExportSelected;
};

MPKFExporter::MPKFExporter() :
	ExportOptions(),
	MaxHWND{ nullptr },
	IGameExporter{ nullptr }
{}

MPKFExporter::~MPKFExporter()
{

}

INT_PTR CALLBACK KFExportDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	MPKFExporter* Exporter = DLGetWindowLongPtr<MPKFExporter*>(hWnd);
	
	ISpinnerControl* SampleValueSpin{ nullptr };
	ISpinnerControl* ScaleValueSpin{ nullptr };
	ISpinnerControl* StartFrameSpin{ nullptr };
	ISpinnerControl* MinPosSpin{ nullptr };
	ISpinnerControl* MinRotSpin{ nullptr };
	ISpinnerControl* EndFrameSpin{ nullptr };
	ISpinnerControl* LoopToFrameSpin{ nullptr };
		
	switch(message) {
		case WM_INITDIALOG:
			Exporter = (MPKFExporter*)lParam;
			DLSetWindowLongPtr(hWnd, lParam);
			ScaleValueSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_SCALE_SPIN));
			ScaleValueSpin->LinkToEdit(GetDlgItem(hWnd, IDC_KF_EXPORTER_SCALE_SPIN_EDIT), EDITTYPE_FLOAT);
			ScaleValueSpin->SetLimits(0.f, 100.f, TRUE);
			ScaleValueSpin->SetScale(0.1f);
			ScaleValueSpin->SetValue(0.01f, FALSE);
			ReleaseISpinner(ScaleValueSpin);

			SampleValueSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_SAMPLE_SPIN));
			SampleValueSpin->LinkToEdit(GetDlgItem(hWnd, IDC_KF_EXPORTER_SAMPLE_SPIN_EDIT), EDITTYPE_INT);
			SampleValueSpin->SetLimits(1, 500, TRUE);
			SampleValueSpin->SetScale(1);
			SampleValueSpin->SetValue(30, FALSE);
			ReleaseISpinner(SampleValueSpin);

			StartFrameSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_START_FRAME_SPIN));
			StartFrameSpin->LinkToEdit(GetDlgItem(hWnd, IDC_KF_EXPORTER_START_FRAME_SPIN_EDIT), EDITTYPE_INT);
			StartFrameSpin->SetLimits(0, 100, TRUE);
			StartFrameSpin->SetScale(1);
			StartFrameSpin->SetValue(0, FALSE);
			ReleaseISpinner(StartFrameSpin);

			EndFrameSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_END_FRAME_SPIN));
			EndFrameSpin->LinkToEdit(GetDlgItem(hWnd, IDC_KF_EXPORTER_END_FRAME_SPIN_EDIT), EDITTYPE_INT);
			EndFrameSpin->SetLimits(0, 100, TRUE);
			EndFrameSpin->SetScale(1);
			EndFrameSpin->SetValue(100, FALSE);
			ReleaseISpinner(EndFrameSpin);

			MinPosSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_MIN_POS_SPIN));
			MinPosSpin->LinkToEdit(GetDlgItem(hWnd, IDC_KF_EXPORTER_MIN_POS_SPIN_EDIT), EDITTYPE_FLOAT);
			MinPosSpin->SetLimits(0.f, 100.f, TRUE);
			MinPosSpin->SetScale(0.1f);
			MinPosSpin->SetValue(0.001f, FALSE);
			ReleaseISpinner(MinPosSpin);
			
			MinRotSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_MIN_ROT_SPIN));
			MinRotSpin->LinkToEdit(GetDlgItem(hWnd, IDC_KF_EXPORTER_MIN_ROT_SPIN_EDIT), EDITTYPE_FLOAT);
			MinRotSpin->SetLimits(0.f, 100.f, TRUE);
			MinRotSpin->SetScale(0.1f);
			MinRotSpin->SetValue(0.001f, FALSE);
			ReleaseISpinner(MinRotSpin);

			LoopToFrameSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_LOOP_TO_FRAME_SPIN));
			LoopToFrameSpin->LinkToEdit(GetDlgItem(hWnd, IDC_KF_EXPORTER_LOOP_TO_FRAME_SPIN_EDIT), EDITTYPE_INT);
			LoopToFrameSpin->SetLimits(0, 100, TRUE);
			LoopToFrameSpin->SetScale(1);
			LoopToFrameSpin->SetValue(0, FALSE);
			ReleaseISpinner(LoopToFrameSpin);

			SendMessage(GetDlgItem(hWnd, IDC_KF_EXPORTER_INTERPOLATION_VALUE), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T("None")));
			SendMessage(GetDlgItem(hWnd, IDC_KF_EXPORTER_INTERPOLATION_VALUE), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T("Orhonormalized")));
			SendMessage(GetDlgItem(hWnd, IDC_KF_EXPORTER_INTERPOLATION_VALUE), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T("Normalized")));

			ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_KF_EXPORTER_INTERPOLATION_VALUE), 0);

			SendMessage(GetDlgItem(hWnd, IDC_KF_EXPORTER_PATHS), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T("textures;..\\sharedtextures")));
			ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_KF_EXPORTER_PATHS), 0);

			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_MP1), TRUE);

			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_USE_GLOBAL_ANIMATION), TRUE);
			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_ANIMATIONS), TRUE);
			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_MATERIALS), TRUE);
			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_FLOATING_VERTICES), TRUE);
			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_HIDDEN_OBJECTS), TRUE);
			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_SCALE), TRUE);
			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_RETAIN_HIERARCHIES), TRUE);
			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_USE_PIVOT), TRUE);
			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_SAVE_REFS), TRUE);
			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_GEOMETRY), TRUE);

			EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_LOOP_INTERP), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_LOOP_DEC_FRAME), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_MAINTAIN_MATRIX), FALSE);

			Edit_SetText(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), _T("_skin.skd"));
			EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), FALSE);

			//Start unsupporter stuff
			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_SAVE_REFS), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_SAVE_REFS), FALSE);

			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_USE_PIVOT), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_USE_PIVOT), FALSE);

			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_FLOATING_VERTICES), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_FLOATING_VERTICES), FALSE);
			
			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_CAMERAS), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_CAMERAS), FALSE);

			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_LIGHTS), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_LIGHTS), FALSE);

			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_ENVIRONMENT), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_ENVIRONMENT), FALSE);

			Button_SetCheck(GetDlgItem(hWnd, IDC_KF_EXPORTER_HELPERS), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_HELPERS), FALSE);
			//End unsuported stuff

			return TRUE;
		case WM_COMMAND:
			switch (HIWORD(wParam)) {
				case CBN_SELCHANGE:
				{
					if (LRESULT SelectedItemIndex = ComboBox_GetCurSel(GetDlgItem(hWnd, IDC_KF_EXPORTER_INTERPOLATION_VALUE));  SelectedItemIndex <= 0) {
						EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_MAINTAIN_MATRIX), FALSE);
					}
					else {
						EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_MAINTAIN_MATRIX), TRUE);
					}
					
					return TRUE;
				}
			}
			switch (LOWORD(wParam)) {
			case IDC_KF_EXPORTER_LOOP:
				if (IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_LOOP) == BST_CHECKED) {
					EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_LOOP_INTERP), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_LOOP_DEC_FRAME), TRUE);
				}
				else {
					EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_LOOP_INTERP), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_LOOP_DEC_FRAME), FALSE);
				}
				return TRUE;
			case IDC_KF_EXPORTER_SCALE:
				if (IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_SCALE) == BST_CHECKED) {
					ScaleValueSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_SCALE_SPIN));
					ScaleValueSpin->Enable(TRUE);
					ReleaseISpinner(ScaleValueSpin);
				}
				else {
					ScaleValueSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_SCALE_SPIN));
					ScaleValueSpin->Enable(FALSE);
					ReleaseISpinner(ScaleValueSpin);
				}
				return TRUE;
			case IDC_KF_EXPORTER_GEOMETRY:
				if (IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_GEOMETRY) == BST_CHECKED) {
					EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING), TRUE);
					if (IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_SKINNING) == BST_CHECKED) {
						EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), TRUE);
					}
					else {
						EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), FALSE);
					}
				}
				else {
					EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING), FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), FALSE);
				}
				return TRUE;
			case IDC_KF_EXPORTER_SKINNING:
				if (IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_SKINNING) == BST_CHECKED) {
					EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), TRUE);
				}
				else {
					EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), FALSE);
				}
				return TRUE;
			case IDOK: {
				MPKFExporterOptions* Options = Exporter->GetExportOptions();
				Options->ExportGeometry = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_GEOMETRY) == BST_CHECKED;
				Options->ExportSkinning = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_SKINNING) == BST_CHECKED;
				Options->ExportAnimations = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_ANIMATIONS) == BST_CHECKED;
				Options->ExportMaterials = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_MATERIALS) == BST_CHECKED;
				Options->ExportCameras = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_CAMERAS) == BST_CHECKED;
				Options->ExportLights = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_LIGHTS) == BST_CHECKED;
				Options->ExportEnvironmets = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_ENVIRONMENT) == BST_CHECKED;
				Options->ExportHelpers = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_HELPERS) == BST_CHECKED;
				Options->Scale = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_SCALE) == BST_CHECKED;
				Options->CopyTexturesToExportPath = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_COPY_TEXTURES) == BST_CHECKED;
				Options->RetainHierarchies = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_RETAIN_HIERARCHIES) == BST_CHECKED;
				Options->UsePivotAsMeshCenter = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_USE_PIVOT) == BST_CHECKED;
				Options->SaveReferencesOnlyOnce = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_SAVE_REFS) == BST_CHECKED;
				Options->RemoveFloatingVertices = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_FLOATING_VERTICES) == BST_CHECKED;
				Options->RemoveHiddenObjects = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_HIDDEN_OBJECTS) == BST_CHECKED;
				Options->UsePivotAsMeshCenter = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_USE_PIVOT) == BST_CHECKED;
				Options->Game = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_MP1) == BST_CHECKED ? 0 : 1;
				ScaleValueSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_SCALE_SPIN));
				Options->ScaleValue = ScaleValueSpin->GetFVal();
				ReleaseISpinner(ScaleValueSpin);

				Options->UseGlobalAnimationRange = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_USE_GLOBAL_ANIMATION) == BST_CHECKED;
				Options->MaintainMatrixScaling = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_MAINTAIN_MATRIX) == BST_CHECKED;
				Options->LoopWhenFinished = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_LOOP) == BST_CHECKED;
				Options->LoopInterpolation = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_LOOP_INTERP) == BST_CHECKED;
				Options->DecEndFrameByOne = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_LOOP_DEC_FRAME) == BST_CHECKED;

				SampleValueSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_SAMPLE_SPIN));
				Options->SampleRate = SampleValueSpin->GetIVal();
				ReleaseISpinner(SampleValueSpin);

				StartFrameSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_START_FRAME_SPIN));
				Options->StartFrame = StartFrameSpin->GetIVal();
				ReleaseISpinner(StartFrameSpin);

				EndFrameSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_END_FRAME_SPIN));
				Options->EndFrame = EndFrameSpin->GetIVal();
				ReleaseISpinner(EndFrameSpin);

				MinPosSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_MIN_POS_SPIN));
				Options->MinDeltaPosition = MinPosSpin->GetFVal();
				ReleaseISpinner(MinPosSpin);

				MinRotSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_MIN_ROT_SPIN));
				Options->MinDeltaRotation = MinRotSpin->GetFVal();
				ReleaseISpinner(MinRotSpin);

				LoopToFrameSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_LOOP_TO_FRAME_SPIN));
				Options->LoopToFrame = LoopToFrameSpin->GetIVal();
				ReleaseISpinner(LoopToFrameSpin);

				if (LRESULT SelectedItemIndex = SendMessage(GetDlgItem(hWnd, IDC_KF_EXPORTER_INTERPOLATION_VALUE), CB_GETCURSEL, 0, 0);  SelectedItemIndex >= 0) {
					Options->FrameToFrameRotationInterpolation = SelectedItemIndex;
				}
				else {
					Options->FrameToFrameRotationInterpolation = 0;
				}
			
				LRESULT TexturePathStringLen = ComboBox_GetTextLength(GetDlgItem(hWnd, IDC_KF_EXPORTER_PATHS));
				TCHAR* ComboboxText = new TCHAR[TexturePathStringLen + 1];
				ComboBox_GetText(GetDlgItem(hWnd, IDC_KF_EXPORTER_PATHS), ComboboxText, TexturePathStringLen + 1);
				Options->Paths = TSTR(ComboboxText);

				if (!IsASCII(Options->Paths.data())) {
					MaxSDK::MaxMessageBox(hWnd, _T("Path contains non-ASCII characters, only english characters is allowed"), _T("Error"), MB_OK | MB_ICONERROR);
					break;
				}

				LRESULT SkinningPathStringLen = Edit_GetTextLength(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX));
				TCHAR* SkinningText = new TCHAR[SkinningPathStringLen + 1];
				ComboBox_GetText(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), SkinningText, SkinningPathStringLen + 1);
				Options->SkinningPostfix = TSTR(SkinningText);

				if (Options->SkinningPostfix.Length() == 0 || !IsASCII(Options->SkinningPostfix.data())) {
					MaxSDK::MaxMessageBox(hWnd, _T("Skin file postfix contains non-ASCII characters or empty, only english characters is allowed"), _T("Error"), MB_OK | MB_ICONERROR);
					break;
				}

				EndDialog(hWnd, 1);
				break;
			}
			case IDCANCEL:
				EndDialog(hWnd, 0);
				break;
			}
            return TRUE;
		default:
			return FALSE;
	}

	return TRUE;
}

int MPKFExporter::DoExport(const MCHAR *name, ExpInterface *ei, Interface *i, BOOL suppressPrompts, DWORD options)
{
	MaxHWND = i->GetMAXHWnd();

	ExportOptions.ResetToDefault();

    if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_KF_EXPORTER_DIALOG), MaxHWND, KFExportDlgProc, (LPARAM)this)) {
		return IMPEXP_CANCEL;
	}

	MPKFGlobalExporterContext GlobalContext{};

	GlobalContext.ExportSelected = (options & SCENE_EXPORT_SELECTED) ? true : false;

    IGameExporter = GetIGameInterface();

	IGameConversionManager* cm = GetConversionManager();
	cm->SetCoordSystem(IGameConversionManager::CoordSystem::IGAME_D3D);

	IGameExporter->InitialiseIGame(GlobalContext.ExportSelected);
	IGameExporter->SetStaticFrame(0);

	TSTR FilePath, FileName, FileExt;
	SplitFilename(TSTR(name), &FilePath, &FileName, &FileExt);

	const int TopLevelNodesNum = IGameExporter->GetTopLevelNodeCount();

	for (int TopLevelNodeIndex{ 0 }; TopLevelNodeIndex < TopLevelNodesNum; TopLevelNodeIndex++)
	{
		IGameNode* Node = IGameExporter->GetTopLevelNode(TopLevelNodeIndex);

		if (!PrepareNodesList(Node, &GlobalContext)) {
			return IMPEXP_FAIL;
		}
	}

	if (!DoExportSubNodes(&GlobalContext)) {
		return IMPEXP_FAIL;
	}

	if (ExportOptions.ExportMaterials) {
		if (!DoExportKFMaterials(&GlobalContext, FilePath)) {
			return IMPEXP_FAIL;
		}

		GlobalContext.TargetMemoryWriter.Write(GlobalContext.MaterialMemoryWriter.GetData(), GlobalContext.MaterialMemoryWriter.GetSize());
	}

	GlobalContext.TargetMemoryWriter.Write(GlobalContext.OtherMemoryWriter.GetData(), GlobalContext.OtherMemoryWriter.GetSize());

	if (ExportOptions.ExportAnimations) {
		GlobalContext.TargetMemoryWriter.Write(GlobalContext.AnimationMemoryWriter.GetData(), GlobalContext.AnimationMemoryWriter.GetSize());
	}

	FILE* OutputFile = _tfopen(name, _T("wb"));

	fwrite(GlobalContext.TargetMemoryWriter.GetData(), GlobalContext.TargetMemoryWriter.GetSize(), 1, OutputFile);

	fclose(OutputFile);

	if (ExportOptions.ExportSkinning && ExportOptions.ExportGeometry) 
	{
		TSTR SkinFile = FileName + ExportOptions.SkinningPostfix;

		FILE* OutputFile = _tfopen(SkinFile.data(), _T("wb"));

		fwrite(GlobalContext.SkinMemoryWriter.GetData(), GlobalContext.SkinMemoryWriter.GetSize(), 1, OutputFile);

		fclose(OutputFile);
	}

	MaxSDK::MaxMessageBox(GetMaxHWND(), _T("Exported"), _T("Info"), MB_OK | MB_ICONINFORMATION);

	return IMPEXP_SUCCESS;
}

IGameNode* MPKFExporter::GetParentNode(IGameNode* Node)
{
	IGameNode* ParentNode = Node->GetNodeParent();

	if (!ParentNode) {
		return nullptr;
	}

	if (ParentNode->IsNodeHidden() && ExportOptions.RemoveHiddenObjects) {
		return GetParentNode(ParentNode);
	}

	if (ParentNode->IsTarget()) {
		return GetParentNode(ParentNode);
	}

	if (ParentNode->IsGroupOwner()) {
		return GetParentNode(ParentNode);
	}

	return ParentNode;
}

bool MPKFExporter::PrepareNodesList(IGameNode* Node, MPKFGlobalExporterContext* GlobalContext)
{
	bool Export = true;

	if (Node->IsTarget()) {
		Export = false;
	}

	IGameNode* ParentNode = GetParentNode(Node);
	if (Node->IsNodeHidden() && ExportOptions.RemoveHiddenObjects) {
		Export = false;
	}

	IGameObject* GameObject = Node->GetIGameObject();
	IGameObject::ObjectTypes Type = GameObject->GetIGameType();

	switch (Type)
	{
	case IGameObject::IGAME_MESH: {
		if (!ExportOptions.ExportGeometry && !ExportOptions.ExportSkinning && !ExportOptions.ExportAnimations) {
			Export = false;
		}
		break;
	}
	case IGameObject::IGAME_CAMERA:
		if (!ExportOptions.ExportCameras) {
			Export = false;
		}
		break;
	case IGameObject::IGAME_LIGHT:
		if (!ExportOptions.ExportLights) {
			Export = false;
		}
		break;
	case IGameObject::IGAME_HELPER:
		if (!ExportOptions.ExportHelpers) {
			Export = false;
		}
		break;
	case IGameObject::IGAME_BONE:
		Export = false;
		break;
	default:
		SHOW_ERROR(MPKFEXPORTER_ERROR_UNSUPPORTED_OBJECTS, Node->GetName());
		Node->ReleaseIGameObject();
		return false;
	}

	GlobalContext->Nodes.push_back({ Node, ParentNode, Export, Type });

	Node->ReleaseIGameObject();

	const int NuberOfSubNodes = Node->GetChildCount();
	for (int NodeIndex = 0; NodeIndex < NuberOfSubNodes; NodeIndex++) {
		IGameNode* ChildNode = Node->GetNodeChild(NodeIndex);
		if (!PrepareNodesList(ChildNode, GlobalContext)) {
			return false;
		}
	}

	return true;
}

bool MPKFExporter::DoExportSubNodes(MPKFGlobalExporterContext* GlobalContext)
{
	for (const MPKFNodeInfo& NodeInfo : GlobalContext->Nodes) {
		if (NodeInfo.Export && !DoExportNode(NodeInfo.Node, GlobalContext)) {
			return false;
		}
	}
	
	return true;
}

bool MPKFExporter::DoExportNode(IGameNode* Node, MPKFGlobalExporterContext* GlobalContext)
{
	IGameObject* GameObject = Node->GetIGameObject();

	MPKFMeshExporterContext Context{};

	switch (GameObject->GetIGameType())
	{
	case IGameObject::IGAME_MESH: {
		IGameMesh* Mesh = (IGameMesh*)GameObject;

		if (!PrepareKFMeshExportContext(Node, Mesh, GameObject, GlobalContext, &Context)) {
			return false;
		}
	
		if (ExportOptions.ExportGeometry) {
			std::unique_ptr<MPMemoryChunkWriter> MeshChunk{ GlobalContext->OtherMemoryWriter.CreateChunk(0x0C, MPKFTYPE_MESH_CHUNK_ID, 2) };
			if (!DoExportKFMesh(Node, &Context, MeshChunk.get())) {
				Node->ReleaseIGameObject();
				return false;
			}
			GlobalContext->OtherMemoryWriter << MeshChunk.get();
			MeshChunk.reset();
		}

		if (ExportOptions.ExportAnimations) {
			if (!DoExportKFAnimation(Node, &Context, GlobalContext)) {
				Node->ReleaseIGameObject();
				return false;
			}
		}

		if (ExportOptions.ExportSkinning && ExportOptions.ExportGeometry) {
			std::unique_ptr<MPMemoryChunkWriter> SkinChunk{ GlobalContext->SkinMemoryWriter.CreateChunk(0x0C, MPKFTYPE_SKIN_CHUNK_ID, 1) };
			if (!DoExportKFSkinning(&Context, SkinChunk.get())) {
				return false;
			}
			GlobalContext->SkinMemoryWriter << SkinChunk.get();
			SkinChunk.reset();
		}
		break;
	}
	case IGameObject::IGAME_CAMERA:
		break;
	case IGameObject::IGAME_LIGHT:
		break;
	case IGameObject::IGAME_HELPER:
		break;
	}

	Node->ReleaseIGameObject();

	return true;
}

bool MPKFExporter::DoExportKFNode(IGameNode* Node, MPMemoryChunkWriter* ChunkWriter)
{
	WRITE_ASCII_STRING(NodeName, Node->GetName(), ChunkWriter);

	IGameNode* ParentNode = Node->GetNodeParent();
	if (ParentNode && ExportOptions.RetainHierarchies) {
		WRITE_ASCII_STRING(ParentNodeName, ParentNode->GetName(), ChunkWriter);
	}
	else {
		*ChunkWriter << MPString(nullptr, 0);
	}

	GMatrix LocalMatrix = Node->GetLocalTM();
	if (!ExportOptions.RetainHierarchies) {
		LocalMatrix = Node->GetWorldTM();
	}

	Point4 Row1 = LocalMatrix.GetRow(0);
	Point4 Row2 = LocalMatrix.GetRow(1);
	Point4 Row3 = LocalMatrix.GetRow(2);
	Point4 Row4 = LocalMatrix.GetRow(3); 

	MPMatrix4x3 NodeMatrix(Row1.x, Row1.y, Row1.z, Row2.x, Row2.y, Row2.z, Row3.x, Row3.y, Row3.z, Row4.x, Row4.y, Row4.z);

	*ChunkWriter << NodeMatrix;

	if (ParentNode && ExportOptions.RetainHierarchies) {
		*ChunkWriter << true;
	}
	else {
		*ChunkWriter << false;
	}

	*ChunkWriter << MPString(nullptr, 0);

	return true;
}

bool MPKFExporter::DoExportKFGeometry(MPKFMeshExporterContext* Context, MPMemoryChunkWriter* ChunkWriter)
{
	*ChunkWriter << static_cast<uint32_t>(Context->MeshVertices.size());

	for (const MPVector3& Vertex : Context->MeshVertices) {
		ChunkWriter->Write(&Vertex.X, sizeof(float));
		ChunkWriter->Write(&Vertex.Y, sizeof(float));
		ChunkWriter->Write(&Vertex.Z, sizeof(float));
	}

	for (const MPVector3& Vertex : Context->MeshNormals) {
		ChunkWriter->Write(&Vertex.X, sizeof(float));
		ChunkWriter->Write(&Vertex.Y, sizeof(float));
		ChunkWriter->Write(&Vertex.Z, sizeof(float));
	}

	*ChunkWriter << static_cast<uint32_t>(Context->MeshVerticesPerPrimitive.size());

	for (uint32_t NumberVertices : Context->MeshVerticesPerPrimitive) {
		*ChunkWriter << NumberVertices;
	}

	return true;
}

bool MPKFExporter::DoExportKFPolygons(MPKFMeshExporterContext* Context, MPMemoryChunkWriter* ChunkWriter)
{
	*ChunkWriter << static_cast<uint32_t>(Context->MeshIndices.size());

	for (const uint16_t Index : Context->MeshIndices) {
		ChunkWriter->Write(&Index, sizeof(uint16_t));
	}

	*ChunkWriter << static_cast<uint32_t>(Context->MeshIndicesPerPrimitive.size());

	for (const uint16_t NumIndices : Context->MeshIndicesPerPrimitive) {
		*ChunkWriter << static_cast<uint32_t>(NumIndices / 3);
	}

	return true;
}

bool MPKFExporter::DoExportKFSkinning(MPKFMeshExporterContext* Context, MPMemoryChunkWriter* ChunkWriter)
{
	if (!ExportOptions.ExportSkinning || !Context->HasSkin) {
		return true;
	}

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(1);
	WRITE_ASCII_STRING(SkinObjectName, Context->SkinObjectName.data(), ChunkWriter);

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(Context->SkinBones.size());
	for (IGameNode* Bone : Context->SkinBones) {
		WRITE_ASCII_STRING(SkinBoneName, Bone->GetName(), ChunkWriter);
	}

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(Context->SkinVertexOffset.size());
	for (int VertexOffset : Context->SkinVertexOffset) {
		*ChunkWriter << static_cast<int32_t>(VertexOffset);
	}

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(Context->SkinBonesNumPerVertex.size());
	for (int BonesNum : Context->SkinBonesNumPerVertex) {
		*ChunkWriter << static_cast<int32_t>(BonesNum);
	}

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(Context->SkinBonesIDsPerVertex.size());
	for (int BoneID : Context->SkinBonesIDsPerVertex) {
		*ChunkWriter << static_cast<int32_t>(BoneID);
	}

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(Context->SkinWeightsPerVertex.size());
	for (float Weight : Context->SkinWeightsPerVertex) {
		*ChunkWriter << Weight;
	}

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(Context->MeshVerticesPerPrimitive.size());
	for (uint32_t VerticesNum : Context->MeshVerticesPerPrimitive) {
		*ChunkWriter << VerticesNum;
	}

	ChunkWriter->WriteTag(0x1C);
	*ChunkWriter << static_cast<int32_t>(Context->MeshVerticesPerPrimitive.size());
	uint32_t StartVertexIndex = 0;
	for (uint32_t VerticesNum : Context->MeshVerticesPerPrimitive) {
		*ChunkWriter << StartVertexIndex;
		StartVertexIndex += VerticesNum;
	}

	return true;
}

bool MPKFExporter::DoExportKFPolygonMaterials(IGameNode* Node, MPKFMeshExporterContext* Context, MPMemoryChunkWriter* ChunkWriter)
{
	*ChunkWriter << static_cast<int32_t>(Context->MaterialPerPrimitive.size());

	for (IGameMaterial* Material : Context->MaterialPerPrimitive) {
		if (Material == nullptr) {
			SHOW_ERROR(MPKFEXPORTER_ERROR_OBJECTS_WITHOUT_MATERIAL, Node->GetName());
			return false;
		}
		WRITE_ASCII_STRING(MaterialName, Material->GetMaterialName(), ChunkWriter);
	}

	return true;
}

bool MPKFExporter::DoExportKFUVWMapping(MPKFMeshExporterContext* Context, MPMemoryChunkWriter* ChunkWriter)
{
	*ChunkWriter << static_cast<uint32_t>(0);

	*ChunkWriter << static_cast<uint32_t>(Context->MeshTexVertices.size());

	for (MPVector2& Vertex : Context->MeshTexVertices) {
		float Z = 0.f;
		ChunkWriter->Write(&Vertex.X, sizeof(float));
		ChunkWriter->Write(&Vertex.Y, sizeof(float));
		ChunkWriter->Write(&Z, sizeof(float));
	}

	*ChunkWriter << static_cast<uint32_t>(Context->MeshVerticesPerPrimitive.size());

	for (uint32_t NumberVertices : Context->MeshVerticesPerPrimitive) {
		*ChunkWriter << NumberVertices;
	}

	return true;
}

bool MPKFExporter::BuildSkinBoneTree(IGameNode* Node, MPKFMeshExporterContext* Context)
{
	INode* MaxNode = Node->GetMaxNode();
	if (MaxNode->UserPropExists(_T("MP_BONE_HELPER"))) {
		return true;
	}

	IS_ASCII_STRING(Node->GetName());
	
	Context->SkinBones.push_back(Node);

	for (int NodeID = 0; NodeID < Node->GetChildCount(); NodeID++) {
		if (!BuildSkinBoneTree(Node->GetNodeChild(NodeID), Context)) {
			return false;
		}
	}

	return true;
}

INode* GetBoneParentNode(INode* Node, MPKFMeshExporterContext* Context, int& BoneIndex)
{
	INode* BoneNode = Node;

	bool Found = false;
	while (BoneNode != nullptr && !Found) {
		for (size_t BoneNameIndex = 0; BoneNameIndex < Context->SkinBones.size(); BoneNameIndex++) {
			if (TSTR(Context->SkinBones[BoneNameIndex]->GetName()) == TSTR(BoneNode->GetName())) {
				Found = true;
				BoneIndex = BoneNameIndex;
				break;
			}
		}

		if (!Found) {
			BoneNode = BoneNode->GetParentNode();
		}
	}

	return BoneNode;
}

bool MPKFExporter::PrepareKFMeshExportAnimationContext(IGameNode* Node, IGameMesh* Mesh, IGameObject* GameObject, MPKFGlobalExporterContext* GlobalContext, MPKFMeshExporterContext* Context)
{
	if (!ExportOptions.ExportAnimations) {
		return true;
	}

	IGameControl* AnimControl = Node->GetIGameControl();

	IGameKeyTab TMKeys;
	if (!AnimControl->GetFullSampledKeys(TMKeys, 1, IGameControlType::IGAME_TM, true)) {
		return false;
	}

	MPKFAnimationExporterContext Animation;

	Animation.ObjectName = TSTR(Node->GetName());
	Animation.ParentName = _T("");

	if (Node->GetNodeParent() != nullptr) {
		Animation.ParentName = TSTR(Node->GetNodeParent()->GetName());
	}

	Animation.SampleRate = ExportOptions.SampleRate;
	Animation.LoopWhenFinished = ExportOptions.LoopWhenFinished;
	Animation.LoopInterpolation = ExportOptions.LoopInterpolation;
	Animation.LoopToFrame = ExportOptions.LoopToFrame;
	Animation.FrameToFrameRotationInterpolation = ExportOptions.FrameToFrameRotationInterpolation;
	Animation.NumTotalKeyFrames = ExportOptions.EndFrame - ExportOptions.StartFrame;
	Animation.MaintainMatrixScaling = ExportOptions.MaintainMatrixScaling;

	int SceneTicks = IGameExporter->GetSceneTicks();

	uint32_t ActualFrame{ 0 };

	Point3 PrevFramePos;
	Quat PrevFrameRot;

	int PrevFrameID = -1;

	for (int KeyIndex = 0; KeyIndex < TMKeys.Count(); KeyIndex++) {
		
		IGameKey Key = TMKeys[KeyIndex];

		int FrameID = Key.t / 180;

		if (FrameID == PrevFrameID) {
			continue;
		}

		PrevFrameID = FrameID;

		if (FrameID >= ExportOptions.StartFrame && FrameID <= ExportOptions.EndFrame) {
			GMatrix Matrix = Key.sampleKey.gval;

			if (ActualFrame == 0) {
				Point3 PrevFramePos = Matrix.Translation();
				Quat Rot = Matrix.Rotation();
				//Rot.GetEuler(&PrevFrameRot.x, &PrevFrameRot.y, PrevFrameRot.z);
				Animation.Frames.push_back({ ActualFrame , Key.sampleKey.gval });
				ActualFrame++;
				continue;
			}

			Point3 Pos = Matrix.Translation();
			Quat Rot = Matrix.Rotation();

			if (Rot.Equals(PrevFrameRot, ExportOptions.MinDeltaRotation) && Pos.Equals(PrevFramePos, ExportOptions.MinDeltaPosition)) {
				ActualFrame++;
				continue;
			}

			PrevFramePos = Pos;
			PrevFrameRot = Rot;

			Animation.Frames.push_back({ ActualFrame , Matrix });
			ActualFrame++;
		}
	}

	Context->Animations.push_back(Animation);

	return true;
}

bool MPKFExporter::PrepareKFMeshExportSkinContext(IGameNode* Node, IGameMesh* Mesh, IGameObject* GameObject, MPKFGlobalExporterContext* GlobalContext, MPKFMeshExporterContext* Context)
{
	IGameSkin* Skin = GameObject->GetIGameSkin();

	if (!Skin && ExportOptions.ExportSkinning) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NOT_FOUND);
		return false;
	}

	if (!Skin) {
		Context->HasSkin = false;
		return true;
	}

	if (Skin->GetTotalSkinBoneCount() == 0) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NULL_BONES);
		return false;
	}

	if (Mesh->GetNumberOfVerts() != Skin->GetNumOfSkinnedVerts()) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_FREE_VERTEX, Node->GetName());
		return false;
	}

	IGameNode* RootBone = nullptr;
	for (int BoneID = 0; BoneID < Skin->GetTotalBoneCount(); BoneID++) {
		IGameNode* Bone = Skin->GetIGameBone(BoneID, true);
		if (Bone == nullptr) {
			SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NULL_BONE, BoneID);
			return false;
		}
		while (Bone != nullptr) {
			if (Bone->GetNodeParent() == nullptr) {
				if (RootBone != nullptr && RootBone != Bone) {
					SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_MORE_THAN_ONE_ROOT);
					return false;
				}
				RootBone = Bone;
				break;
			}
			Bone = Bone->GetNodeParent();
		}
	}

	if (!BuildSkinBoneTree(RootBone, Context)) {
		return false;
	}

	if (ExportOptions.ExportAnimations) {
		for (IGameNode* Bone : Context->SkinBones) {
			if (!PrepareKFMeshExportAnimationContext(Bone, Mesh, GameObject, GlobalContext, Context)) {
				return false;
			}
		}
	}

	if (!ExportOptions.ExportSkinning) {
		return true;
	}

	Context->HasSkin = true;

	Context->SkinObjectName = TSTR(Node->GetName());

	int SkinVertexOffsetIndex{ 0 };

	for (int VertexIndex : Context->MeshMaxVertexIndex) {
		std::map<int, float> BoneToWeight{};
		float Sum{ 0.f };

		Context->SkinVertexOffset.push_back(SkinVertexOffsetIndex);

		switch (Skin->GetVertexType(VertexIndex)) {
		case IGameSkin::IGAME_RIGID: {

			int BoneNameIndex{ 0 };

			INode* BoneNode = GetBoneParentNode(Skin->GetBone(VertexIndex, 0), Context, BoneNameIndex);
			
			if (BoneNode == nullptr) {
				SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NULL_BONE, VertexIndex);
				return false;
			}

			BoneToWeight[BoneNameIndex] = 1.f;

			Sum = 1.f;

			break;
		}
		case IGameSkin::IGAME_RIGID_BLENDED: {
			int NumberOfBones = Skin->GetNumberOfBones(VertexIndex);

			if (NumberOfBones > 4 && ExportOptions.Game == 1) {
				SHOW_ERROR(MPKFEXPORTER_ERROR_MP2_SKIN_BONES_NUM, Node->GetName());
				return false;
			}

			if (NumberOfBones > 3 && ExportOptions.Game == 0) {
				SHOW_ERROR(MPKFEXPORTER_ERROR_MP1_SKIN_BONES_NUM, Node->GetName());
				return false;
			}

			for (int BoneID = 0; BoneID < NumberOfBones; BoneID++) {
				int BoneNameIndex{ 0 };

				INode* BoneNode = GetBoneParentNode(Skin->GetBone(VertexIndex, BoneID), Context, BoneNameIndex);

				if (BoneNode == nullptr) {
					SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NULL_BONE, VertexIndex);
					return false;
				}

				const float Weight{ Skin->GetWeight(VertexIndex, BoneID) };

				if (BoneToWeight.contains(BoneNameIndex)) {
					BoneToWeight[BoneNameIndex] += Weight;
				}
				else {
					BoneToWeight[BoneNameIndex] = Weight;
				}

				Sum += Weight;
			}

			break;
		}
		default:
			SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_WRONG_VERTEX_TYPE);
			return false;
		}

		for (auto& [BoneId, Weight] : BoneToWeight) {
			Context->SkinBonesIDsPerVertex.push_back(BoneId);
			Context->SkinWeightsPerVertex.push_back(Weight / Sum);
		}

		Context->SkinBonesNumPerVertex.push_back(BoneToWeight.size());
		SkinVertexOffsetIndex += BoneToWeight.size();
	}

	return true;
}

/*bool MPKFExporter::PrepareKFMeshExportSkinContext(IGameNode* Node, IGameMesh* Mesh, IGameObject* GameObject, MPKFGlobalExporterContext* GlobalContext, MPKFMeshExporterContext* Context)
{
	IGameSkin* Skin = GameObject->GetIGameSkin();

	if (!Skin || !ExportOptions.ExportSkinning) {
		Context->HasSkin = false;
		return true;
	}

	Context->HasSkin = true;

	if (Skin->GetTotalSkinBoneCount() == 0) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NULL_BONES);
		return false;
	}

	if (Mesh->GetNumberOfVerts() != Skin->GetNumOfSkinnedVerts()) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_FREE_VERTEX, Node->GetName());
		return false;
	}

	IGameNode* RootBone = nullptr;
	for (int BoneID = 0; BoneID < Skin->GetTotalBoneCount(); BoneID++) {
		IGameNode* Bone = Skin->GetIGameBone(BoneID, true);
		if (Bone == nullptr) {
			SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NULL_BONE, BoneID);
			return false;
		}
		while (Bone != nullptr) {
			if (Bone->GetNodeParent() == nullptr) {
				if (RootBone != nullptr && RootBone != Bone) {
					SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_MORE_THAN_ONE_ROOT);
					return false;
				}
				RootBone = Bone;
				break;
			}
			Bone = Bone->GetNodeParent();
		}
	}

	if (!BuildSkinBoneTree(RootBone, Context)) {
		return false;
	}

	Context->SkinObjectName = TSTR(Node->GetName());

	Modifier* SkinModifier{ Skin->GetMaxModifier() };

	IPhysiqueExport* PhysiqueInterface{ nullptr };
	IPhyContextExport* PhysiqueContext{ nullptr };
	
	if (SkinModifier->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B)) {
		PhysiqueInterface = (IPhysiqueExport*)SkinModifier->GetInterface(I_PHYINTERFACE);
		if (!PhysiqueInterface) {
			SHOW_ERROR(_T("Unable to initialize Physique modifier"));
			return false;
		}

		PhysiqueContext = PhysiqueInterface->GetContextInterface(Node->GetMaxNode());
		if (!PhysiqueContext) {
			SHOW_ERROR(_T("Unable to initialize Physique Context interface"));
			return false;
		}
	}
	else {
		SHOW_ERROR(_T("Unable to initialize Physique modifier"));
		return false;
	}

	PhysiqueContext->AllowBlending(TRUE);
	PhysiqueContext->ConvertToRigid(TRUE);

	int SkinVertexOffsetIndex{ 0 };

	for (int VertexIndex : Context->MeshMaxVertexIndex) {
		std::map<int, float> BoneToWeight{};
		float Sum{ 0.f };

		Context->SkinVertexOffset.push_back(SkinVertexOffsetIndex);

		IPhyVertexExport* PhyVertexInterface = PhysiqueContext->GetVertexInterface(VertexIndex);
		if (!PhyVertexInterface) {
			SHOW_ERROR(_T("Unable to initialize Physique Vertex interface"));
			return false;
		}
		
		if (PhyVertexInterface->GetVertexType() == RIGID_NON_BLENDED_TYPE)
		{
			IPhyRigidVertex* rv = (IPhyRigidVertex*)PhyVertexInterface;

			int BoneNameIndex{ 0 };
			INode* BoneNode = GetBoneParentNode(rv->GetNode(), Context, BoneNameIndex);
			
			if (BoneNode == nullptr) {
				SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NULL_BONE, VertexIndex);
				return false;
			}

			BoneToWeight[BoneNameIndex] = 1.f;

			Sum = 1.f;
		}
		else if (PhyVertexInterface->GetVertexType() == RIGID_BLENDED_TYPE)
		{
			IPhyBlendedRigidVertex* rbv = (IPhyBlendedRigidVertex*)PhyVertexInterface;
			int NumberOfBones = rbv->GetNumberNodes();

			if (NumberOfBones > 4 && ExportOptions.Game == 1) {
				SHOW_ERROR(MPKFEXPORTER_ERROR_MP2_SKIN_BONES_NUM, Node->GetName());
				return false;
			}

			if (NumberOfBones > 3 && ExportOptions.Game == 0) {
				SHOW_ERROR(MPKFEXPORTER_ERROR_MP1_SKIN_BONES_NUM, Node->GetName());
				return false;
			}

			for (int BoneID = 0; BoneID < NumberOfBones; BoneID++) {
				int BoneNameIndex{ 0 };
				INode* BoneNode = GetBoneParentNode(rbv->GetNode(BoneID), Context, BoneNameIndex);

				if (BoneNode == nullptr) {
					SHOW_ERROR(MPKFEXPORTER_ERROR_SKIN_NULL_BONE, VertexIndex);
					return false;
				}

				const float Weight{ rbv->GetWeight(BoneID) };

				if (BoneToWeight.contains(BoneNameIndex)) {
					BoneToWeight[BoneNameIndex] += Weight;
				}
				else {
					BoneToWeight[BoneNameIndex] = Weight;
				}

				Sum += Weight;
			}
		} 
		else 
		{
			SHOW_ERROR(_T("Unknown vertex type in physiqued object"));
			return false;
		}

		PhysiqueContext->ReleaseVertexInterface(PhyVertexInterface);

		for (auto& [BoneId, Weight] : BoneToWeight) {
			Context->SkinBonesIDsPerVertex.push_back(BoneId);
			Context->SkinWeightsPerVertex.push_back(Weight / Sum);
		}

		Context->SkinBonesNumPerVertex.push_back(BoneToWeight.size());
		SkinVertexOffsetIndex += BoneToWeight.size();
	}

	PhysiqueInterface->ReleaseContextInterface(PhysiqueContext);

	return true;
}*/

bool MPKFExporter::PrepareKFMeshExportContext(IGameNode* Node, IGameMesh* Mesh, IGameObject* GameObject, MPKFGlobalExporterContext* GlobalContext, MPKFMeshExporterContext* Context)
{
	Mesh->SetCreateOptimizedNormalList();
	if (!Mesh->InitializeData()) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_MESH_TRIANGULATE, Node->GetName());
		return false;
	}

	std::vector<MPVector3> LocalNormals;
	std::vector<MPVector2> LocalTexVertices;
	std::vector<MPVector3> LocalVertices;
	std::vector<uint16_t> LocalIndices;
	std::map<uint16_t, std::map<uint16_t, std::map<uint16_t, uint16_t>>> MaxVertexToLocal;

	Context->MeshVertices.clear();
	Context->MeshTexVertices.clear();
	Context->MeshNormals.clear();
	Context->MeshIndices.clear();
	Context->MeshVerticesPerPrimitive.clear();
	Context->MeshIndicesPerPrimitive.clear();
	Context->MaterialPerPrimitive.clear();

	std::map<int, std::vector<int>> MaterialGroups;

	for (int FaceID = 0; FaceID < Mesh->GetNumberOfFaces(); FaceID++)
	{
		IGameMaterial* Mat = Mesh->GetMaterialFromFace(FaceID);
		FaceEx* Face = Mesh->GetFace(FaceID);

		auto Result = std::find(std::begin(Context->MaterialPerPrimitive), std::end(Context->MaterialPerPrimitive), Mat);

		if (Result != std::end(Context->MaterialPerPrimitive)) {
			int index = std::distance(std::begin(Context->MaterialPerPrimitive), Result);
			MaterialGroups[index].push_back(FaceID);
		}
		else {
			MaterialGroups[Context->MaterialPerPrimitive.size()].push_back(FaceID);
			Context->MaterialPerPrimitive.push_back(Mat);
		}

		auto MaterialsContextResult = std::find(std::begin(GlobalContext->Materials), std::end(GlobalContext->Materials), Mat);
		if (MaterialsContextResult == std::end(GlobalContext->Materials)) {
			GlobalContext->Materials.push_back(Mat);
		}
	}

	for (auto MaterialIter : MaterialGroups)
	{
		LocalNormals.clear();
		LocalVertices.clear();
		LocalIndices.clear();
		LocalTexVertices.clear();
		MaxVertexToLocal.clear();
	
		for (int FaceID : MaterialIter.second)
		{
			FaceEx* Face = Mesh->GetFace(FaceID);

			for (uint16_t i = 0; i < 3; i++) {
				if (MaxVertexToLocal.contains(Face->vert[i]) &&
					MaxVertexToLocal[Face->vert[i]].contains(Face->norm[i]) &&
					MaxVertexToLocal[Face->vert[i]][Face->norm[i]].contains(Face->texCoord[i]))
				{
					LocalIndices.push_back(MaxVertexToLocal[Face->vert[i]][Face->norm[i]][Face->texCoord[i]]);
					continue;
				}

				MaxVertexToLocal[Face->vert[i]][Face->norm[i]][Face->texCoord[i]] = LocalVertices.size();
				Context->MeshMaxVertexIndex.push_back(Face->vert[i]);
				
				float ScaleValue = 1.f;
				if (ExportOptions.Scale) {
					ScaleValue = ExportOptions.ScaleValue;
				}

				Point2 TexVertex = Mesh->GetTexVertex(Face->texCoord[i]);
				LocalTexVertices.push_back(MPVector2(TexVertex.x, TexVertex.y));

				Point3 Vertex = Mesh->GetVertex(Face->vert[i], true);
				LocalVertices.push_back(MPVector3(Vertex.x * ScaleValue, Vertex.y * ScaleValue, Vertex.z * ScaleValue));

				Point3 Normal = Mesh->GetNormal(Face->norm[i], true);
				LocalNormals.push_back(MPVector3(Normal.x, Normal.y, Normal.z));

				LocalIndices.push_back(MaxVertexToLocal[Face->vert[i]][Face->norm[i]][Face->texCoord[i]]);
			}
		}

		for (MPVector3& Vertex : LocalVertices) {
			Context->MeshVertices.push_back(Vertex);
		}

		for (MPVector3& Normal : LocalNormals) {
			Context->MeshNormals.push_back(Normal);
		}

		for (MPVector2& TexVertex : LocalTexVertices) {
			Context->MeshTexVertices.push_back(TexVertex);
		}

		for (uint16_t Index : LocalIndices) {
			Context->MeshIndices.push_back(Index);
		}

		Context->MeshVerticesPerPrimitive.push_back(LocalVertices.size());

		Context->MeshIndicesPerPrimitive.push_back(LocalIndices.size());
	}

	if (!PrepareKFMeshExportSkinContext(Node, Mesh, GameObject, GlobalContext, Context)) {
		return false;
	}

	return true;
}


bool MPKFExporter::DoExportKFAnimation(IGameNode* Node, MPKFMeshExporterContext* Context, MPKFGlobalExporterContext* GlobalContext)
{
	for (MPKFAnimationExporterContext& Animation : Context->Animations) {

		std::unique_ptr<MPMemoryChunkWriter> AnimationChunk{ GlobalContext->AnimationMemoryWriter.CreateChunk(0x0C, MPKFTYPE_KEYFRAME_ANIMATION_CHUNK_ID, 5) };

		std::unique_ptr<MPMemoryChunkWriter> AnimationSubChunk{ GlobalContext->AnimationMemoryWriter.CreateChunk(0x0C, MPKFTYPE_ANIMATION_SUB_CHUNK_ID, 0) };

		WRITE_ASCII_STRING(ObjectName, Animation.ObjectName.data(), AnimationSubChunk);
		*AnimationSubChunk << Animation.SampleRate;
		*AnimationSubChunk << Animation.LoopWhenFinished;

		*AnimationChunk << AnimationSubChunk.get();

		WRITE_ASCII_STRING(ParentName, Animation.ParentName.data(), AnimationChunk);
		*AnimationChunk << Animation.LoopInterpolation;
		*AnimationChunk << Animation.NumTotalKeyFrames;
		*AnimationChunk << static_cast<uint32_t>(Animation.Frames.size());

		for (const MPKFAnimationFrameExporterContext& Frame : Animation.Frames) {
			*AnimationChunk << Frame.FrameID;

			Point4 Row1 = Frame.Transform.GetRow(0);
			Point4 Row2 = Frame.Transform.GetRow(1);
			Point4 Row3 = Frame.Transform.GetRow(2);
			Point4 Row4 = Frame.Transform.GetRow(3);

			float Scale = 1.f;
			if (ExportOptions.Scale) {
				Scale = ExportOptions.ScaleValue;
			}

			MPMatrix4x3 NodeMatrix(Row1.x, Row1.y, Row1.z, Row2.x, Row2.y, Row2.z, Row3.x, Row3.y, Row3.z, Row4.x * Scale, Row4.y * Scale, Row4.z * Scale);

			*AnimationChunk << NodeMatrix;
		}

		//visibility frames
		*AnimationChunk << static_cast<uint32_t>(1);
		*AnimationChunk << static_cast<uint32_t>(0);
		*AnimationChunk << 1.f;

		*AnimationChunk << Animation.LoopToFrame;
		*AnimationChunk << Animation.FrameToFrameRotationInterpolation;
		*AnimationChunk << Animation.MaintainMatrixScaling;

		GlobalContext->AnimationMemoryWriter << AnimationChunk.get();

		AnimationSubChunk.reset();
		AnimationChunk.reset();
	}

	return true;
}

bool MPKFExporter::DoExportKFMesh(IGameNode* Node, MPKFMeshExporterContext* Context, MPMemoryChunkWriter* ChunkWriter)
{
	//Node
	std::unique_ptr<MPMemoryChunkWriter> NodeChunk{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_NODE_SUB_CHUNK_ID, 1) };
	if (!DoExportKFNode(Node, NodeChunk.get())) {
		return false;
	}
	*ChunkWriter << NodeChunk.get();
	NodeChunk.reset();

	//Geometry
	std::unique_ptr<MPMemoryChunkWriter> GeometryChunk{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_GEOMETRY_SUB_CHUNK_ID, 1) };
	if (!DoExportKFGeometry(Context, GeometryChunk.get())) {
		return false;
	}
	*ChunkWriter << GeometryChunk.get();
	GeometryChunk.reset();

	//Polygons
	std::unique_ptr<MPMemoryChunkWriter> PolygonsChunk{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_POLYGONS_SUB_CHUNK_ID, 1) };
	if (!DoExportKFPolygons(Context, PolygonsChunk.get())) {
		return false;
	}
	*ChunkWriter << PolygonsChunk.get();
	PolygonsChunk.reset();

	//Polygon Materials
	std::unique_ptr<MPMemoryChunkWriter> PolygonMaterialsChunk{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_POLYGON_MATERIAL_SUB_CHUNK_ID, 1) };
	if (!DoExportKFPolygonMaterials(Node, Context, PolygonMaterialsChunk.get())) {
		return false;
	}
	*ChunkWriter << PolygonMaterialsChunk.get();
	PolygonMaterialsChunk.reset();

	//Polygon UVW
	if (ExportOptions.ExportMaterials) {
		std::unique_ptr<MPMemoryChunkWriter> UVChunk{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_UV_MAPPING_SUB_CHUNK_ID, 1) };
		if (!DoExportKFUVWMapping(Context, UVChunk.get())) {
			return false;
		}
		*ChunkWriter << UVChunk.get();
		UVChunk.reset();
	}

	return true;
}

bool MPKFExporter::DoExportKFMaterials(MPKFGlobalExporterContext* GlobalContext, const TSTR& CopyDirTo)
{
	if (!ExportOptions.ExportMaterials) {
		return true;
	}

	std::unique_ptr<MPMemoryChunkWriter> MaterialListChunk{ GlobalContext->MaterialMemoryWriter.CreateChunk(0x0C, MPKFTYPE_MATERIAL_LIST_CHUNK_ID, 0) };

	const int NumberOfRootMaterials = GlobalContext->Materials.size();

	CStr PathsAsc = ExportOptions.Paths.ToCStr();
	*MaterialListChunk << MPString(PathsAsc.data(), PathsAsc.Length());
	*MaterialListChunk << static_cast<int32_t>(GlobalContext->Materials.size());

	for (IGameMaterial* Material : GlobalContext->Materials)
	{
		std::unique_ptr<MPMemoryChunkWriter> MaterialChunk{ GlobalContext->MaterialMemoryWriter.CreateChunk(0x0C, MPKFTYPE_MATERIAL_SUB_CHUNK_ID, ExportOptions.Game == 0 ? 1 : 2) };
		if (!DoExportKFMaterial(Material, CopyDirTo, MaterialChunk.get())) {
			return false;
		}
		*MaterialListChunk << MaterialChunk.get();
	}

	GlobalContext->MaterialMemoryWriter << MaterialListChunk.get();

	return true;
}

bool MPKFExporter::DoExportKFTexture(Texmap* Texture, const TSTR& CopyDirTo, MPMemoryChunkWriter* ChunkWriter)
{
	if (!Texture) {
		return false;
	}

	if (Texture->ClassID() != MPKFTEXTURE_CLASS_ID) {
		TSTR Str;
		Texture->GetClassName(Str);
		SHOW_ERROR(MPKFEXPORTER_ERROR_UNSUPORTED_TEXTURE, Texture->GetName().data(), Str.data());
	}

	std::unique_ptr<MPMemoryChunkWriter> TexureChunkWriter{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_TEXTURE_SUB_CHUNK_ID, 1)};
	IMPKFTexture* KFTexture = dynamic_cast<IMPKFTexture*>(Texture);

	const int32_t NumberOfTextures = KFTexture->GetTexturesCount();

	if (NumberOfTextures <= 0) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_TEXTURE_EMPTY_BITMAP, KFTexture->GetTextureName());
		return false;
	}

	if (NumberOfTextures > 1 && KFTexture->GetAnimationStartFrame() > NumberOfTextures) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_TEXTURE_START_FRAME, KFTexture->GetTextureName(), NumberOfTextures, KFTexture->GetAnimationStartFrame());
		return false;
	}

	WRITE_ASCII_STRING(TextureName, KFTexture->GetTextureName(), TexureChunkWriter);

	if (KFTexture->IsMipMapsAuto()) {
		*TexureChunkWriter << (int32_t)(0);
	} else {
		*TexureChunkWriter << KFTexture->GetMipMapsNum();
	}

	*TexureChunkWriter << static_cast<int32_t>(KFTexture->GetFiltering());
	*TexureChunkWriter << NumberOfTextures;

	for (int32_t TextureIndex = 0; TextureIndex != NumberOfTextures; TextureIndex++) {
		const TSTR TextureFileName = KFTexture->GetTextureFileName(TextureIndex);

		if (TextureFileName == _T("")) {
			SHOW_ERROR(MPKFEXPORTER_ERROR_TEXTURE_EMPTY_BITMAP, KFTexture->GetTextureName());
			return false;
		}

		IS_ASCII_STRING(TextureFileName.data());

		TSTR FilePath, FileName, FileExt;
		SplitFilename(TextureFileName, &FilePath, &FileName, &FileExt);

		TSTR FileNameWithExt(FileName + FileExt);

		if (ExportOptions.CopyTexturesToExportPath) {
			TSTR CopyPath(CopyDirTo);
			CopyPath.Append(_T("\\")).Append(FileNameWithExt);
			CopyFile(TextureFileName, CopyPath.data(), FALSE);
		}

		CStr FileNameAsc = FileNameWithExt.ToCStr();
		*TexureChunkWriter << MPString(FileNameAsc.data(), FileNameAsc.Length());
	}

	if (NumberOfTextures > 1) {
		*TexureChunkWriter << KFTexture->IsAnimationAutomaticStart();
		*TexureChunkWriter << KFTexture->IsAnimationRandomStartFrame();
		*TexureChunkWriter << KFTexture->GetAnimationStartFrame();
		*TexureChunkWriter << KFTexture->GetAnimationFPS();
		*TexureChunkWriter << static_cast<int32_t>(KFTexture->GetAnimationEndCondition());
	}

	*ChunkWriter << TexureChunkWriter.get();

	return true;
}

bool MPKFExporter::DoExportKFMaterial(IGameMaterial* Material, const TSTR& CopyDirTo, MPMemoryChunkWriter* ChunkWriter)
{
	Mtl* MaxMaterial = Material->GetMaxMaterial();
	
	if (MaxMaterial->ClassID() != MPKFMATERIAL_CLASS_ID) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_UNSUPORTED_MATERIAL, Material->GetMaterialName(), Material->GetClassName());
		return false;
	}

	IS_ASCII_STRING(Material->GetMaterialName());

	IMPKFMaterial* KFMaterial = dynamic_cast<IMPKFMaterial*>(MaxMaterial);

	if (static_cast<int>(KFMaterial->GetGameVersion()) != ExportOptions.Game) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_MATERIAL_WRONG_TARGET, ExportOptions.Game == 0 ? _T("Max Payne 1") : _T("Max Payne 2"), Material->GetMaterialName(), KFMaterial->GetGameVersion() == KFMaterialGameVersion::kMaxPayne1 ? _T("Max Payne 1") : _T("Max Payne 2"));
		return false;
	}

	WRITE_ASCII_STRING(MaterialName, Material->GetMaterialName(), ChunkWriter);

	*ChunkWriter << KFMaterial->IsTwoSided();
	*ChunkWriter << KFMaterial->IsFogging();
	*ChunkWriter << false; //is_diffuse_combined
	*ChunkWriter << KFMaterial->IsInvisibleGeometry();
	*ChunkWriter << KFMaterial->HasVertexAlpha();
	*ChunkWriter << static_cast<int32_t>(KFMaterial->GetDiffuseColorShadingType());
	*ChunkWriter << static_cast<int32_t>(KFMaterial->GetSpecularColorShadingType());
	*ChunkWriter << static_cast<int32_t>(KFMaterial->GetLitShadingType());

	MPColor AmbienColor = KFMaterial->GetAmbientColor();
	*ChunkWriter << AmbienColor.R;
	*ChunkWriter << AmbienColor.G;
	*ChunkWriter << AmbienColor.B;
	*ChunkWriter << AmbienColor.A;

	MPColor DiffuseColor = KFMaterial->GetDiffuseColor();
	*ChunkWriter << DiffuseColor.R;
	*ChunkWriter << DiffuseColor.G;
	*ChunkWriter << DiffuseColor.B;
	*ChunkWriter << DiffuseColor.A;
	
	MPColor SpecularColor = KFMaterial->GetSpecularColor();
	*ChunkWriter << SpecularColor.R;
	*ChunkWriter << SpecularColor.G;
	*ChunkWriter << SpecularColor.B;
	*ChunkWriter << SpecularColor.A;

	*ChunkWriter << KFMaterial->GetVertexAlphaValue();
	*ChunkWriter << KFMaterial->GetSpecularExponent();
	*ChunkWriter << static_cast<int32_t>(KFMaterial->GetDiffuseTextureShadingType());
	*ChunkWriter << static_cast<int32_t>(KFMaterial->GetReflectionTextureShadingType());

	if (ExportOptions.Game == 0) {
		*ChunkWriter << KFMaterial->GetBumpEmbossFactor();
	} else {
		*ChunkWriter << 0.f;
	}

	*ChunkWriter << KFMaterial->HasDiffuseTexture();
	if (KFMaterial->HasDiffuseTexture()) {
		if (!KFMaterial->GetDiffuseTexture()) {
			SHOW_ERROR(MPKFEXPORTER_ERROR_TEXTURE_EMPTY, _T("Diffuse"), MaterialName.data());
			return false;
		}
		if (!DoExportKFTexture(KFMaterial->GetDiffuseTexture(), CopyDirTo, ChunkWriter)) {
			return false;
		}
	}

	*ChunkWriter << KFMaterial->HasReflectionTexture();
	if (KFMaterial->HasReflectionTexture()) {
		if (!KFMaterial->GetReflectionTexture()) {
			SHOW_ERROR(MPKFEXPORTER_ERROR_TEXTURE_EMPTY, _T("Reflection"), MaterialName.data());
			return false;
		}
		if (!DoExportKFTexture(KFMaterial->GetReflectionTexture(), CopyDirTo, ChunkWriter)) {
			return false;
		}
	}

	if (ExportOptions.Game == 0) {
		*ChunkWriter << KFMaterial->HasBumpTexture();
		if (KFMaterial->HasBumpTexture()) {
			if (!KFMaterial->GetBumpTexture()) {
				SHOW_ERROR(MPKFEXPORTER_ERROR_TEXTURE_EMPTY, _T("Bump"), MaterialName.data());
				return false;
			}
			if (!DoExportKFTexture(KFMaterial->GetBumpTexture(), CopyDirTo, ChunkWriter)) {
				return false;
			}
		}
	}
	else {
		*ChunkWriter << false;
	}

	if (ExportOptions.Game == 0) {
		*ChunkWriter << KFMaterial->HasAlphaTexture();
		if (KFMaterial->HasAlphaTexture()) {
			if (!KFMaterial->GetAlphaTexture()) {
				SHOW_ERROR(MPKFEXPORTER_ERROR_TEXTURE_EMPTY, _T("Alpha"), MaterialName.data());
				return false;
			}
			if (!DoExportKFTexture(KFMaterial->GetAlphaTexture(), CopyDirTo, ChunkWriter)) {
				return false;
			}
		}
	}
	else {
		*ChunkWriter << false;
	}

	if (ExportOptions.Game == 0) {
		*ChunkWriter << KFMaterial->HasMaskTexture();
		if (KFMaterial->HasMaskTexture()) {
			if (!KFMaterial->GetMaskTexture()) {
				SHOW_ERROR(MPKFEXPORTER_ERROR_TEXTURE_EMPTY, _T("Mask"), MaterialName.data());
				return false;
			}
			if (!DoExportKFTexture(KFMaterial->GetMaskTexture(), CopyDirTo, ChunkWriter)) {
				return false;
			}
		}
	}
	else {
		*ChunkWriter << false;
	}

	*ChunkWriter << static_cast<int32_t>(KFMaterial->GetMaskTextureShadingType());
	*ChunkWriter << KFMaterial->HasLit();

	if (ExportOptions.Game == 1) {
		*ChunkWriter << KFMaterial->HasAlphaCompare();
		*ChunkWriter << KFMaterial->HasEdgeBlend();
		*ChunkWriter << KFMaterial->GetAlphaReferenceValue(); //TODO: INT
	}
	
	return true;
}

