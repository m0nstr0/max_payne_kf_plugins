#include "MPKFExporter.h"

#include <MPMemoryWriter.h>
#include <IMPKFTexture.h>
#include <set>
#include <vector>
#include <map>
#include <string>

#include "MPKFMesh.h"
#include "MPKFScene.h"

#define MPKFEXPORTER_ERROR_UNSUPPORTED_OBJECTS				_T("The scene has an unsupported object \"{}\"")
#define MPKFEXPORTER_ERROR_MP2_SKIN_BONES_NUM				_T("Max Payne 2: Object \"{}\" has more that 4 bones per vertex")
#define MPKFEXPORTER_ERROR_MP1_SKIN_BONES_NUM				_T("Max Payne 1: Object \"{}\" has more that 3 bones per vertex")
#define MPKFEXPORTER_ERROR_SKIN_FREE_VERTEX					_T("There are some vertices have zero weight sum \"{}\"")
#define MPKFEXPORTER_ERROR_SKIN_NULL_BONE					_T("Bone with index \"{}\" not found")
#define MPKFEXPORTER_ERROR_SKIN_NULL_BONES					_T("There are no bones")
#define MPKFEXPORTER_ERROR_SKIN_MORE_THAN_ONE_ROOT			_T("There are more than one root bone")
#define MPKFEXPORTER_ERROR_SKIN_NOT_FOUND					_T("Skin or Physique modifier not found")
#define MPKFEXPORTER_ERROR_SKIN_WRONG_VERTEX_TYPE			_T("Unknown vertex type. Only RIGID and RIGI_BLENDED are supported")

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

HWND gMaxWndHandle;

MPKFExporter::MPKFExporter() :
	GameScene{ nullptr }
{}

INT_PTR CALLBACK KFExportDlgProc(const HWND HWnd, const UINT Message, const WPARAM WParam, const LPARAM LParam)
{
	MPKFExporter* Exporter = DLGetWindowLongPtr<MPKFExporter*>(HWnd);
	
	ISpinnerControl* SampleValueSpin{ nullptr };
	ISpinnerControl* ScaleValueSpin{ nullptr };
	ISpinnerControl* StartFrameSpin{ nullptr };
	ISpinnerControl* MinPosSpin{ nullptr };
	ISpinnerControl* MinRotSpin{ nullptr };
	ISpinnerControl* EndFrameSpin{ nullptr };
	ISpinnerControl* LoopToFrameSpin{ nullptr };
		
	switch(Message) {
		case WM_INITDIALOG:
			Exporter = reinterpret_cast<MPKFExporter*>(LParam);
			DLSetWindowLongPtr(HWnd, LParam);
			ScaleValueSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_SCALE_SPIN));
			ScaleValueSpin->LinkToEdit(GetDlgItem(HWnd, IDC_KF_EXPORTER_SCALE_SPIN_EDIT), EDITTYPE_FLOAT);
			ScaleValueSpin->SetLimits(0.f, 100.f, TRUE);
			ScaleValueSpin->SetScale(0.1f);
			ScaleValueSpin->SetValue(0.01f, FALSE);
			ReleaseISpinner(ScaleValueSpin);

			SampleValueSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_SAMPLE_SPIN));
			SampleValueSpin->LinkToEdit(GetDlgItem(HWnd, IDC_KF_EXPORTER_SAMPLE_SPIN_EDIT), EDITTYPE_INT);
			SampleValueSpin->SetLimits(1, 500, TRUE);
			SampleValueSpin->SetScale(1);
			SampleValueSpin->SetValue(30, FALSE);
			ReleaseISpinner(SampleValueSpin);

			StartFrameSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_START_FRAME_SPIN));
			StartFrameSpin->LinkToEdit(GetDlgItem(HWnd, IDC_KF_EXPORTER_START_FRAME_SPIN_EDIT), EDITTYPE_INT);
			StartFrameSpin->SetLimits(0, 100, TRUE);
			StartFrameSpin->SetScale(1);
			StartFrameSpin->SetValue(0, FALSE);
			ReleaseISpinner(StartFrameSpin);

			EndFrameSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_END_FRAME_SPIN));
			EndFrameSpin->LinkToEdit(GetDlgItem(HWnd, IDC_KF_EXPORTER_END_FRAME_SPIN_EDIT), EDITTYPE_INT);
			EndFrameSpin->SetLimits(0, 100, TRUE);
			EndFrameSpin->SetScale(1);
			EndFrameSpin->SetValue(100, FALSE);
			ReleaseISpinner(EndFrameSpin);

			MinPosSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_MIN_POS_SPIN));
			MinPosSpin->LinkToEdit(GetDlgItem(HWnd, IDC_KF_EXPORTER_MIN_POS_SPIN_EDIT), EDITTYPE_FLOAT);
			MinPosSpin->SetLimits(0.f, 100.f, TRUE);
			MinPosSpin->SetScale(0.1f);
			MinPosSpin->SetValue(0.001f, FALSE);
			ReleaseISpinner(MinPosSpin);
			
			MinRotSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_MIN_ROT_SPIN));
			MinRotSpin->LinkToEdit(GetDlgItem(HWnd, IDC_KF_EXPORTER_MIN_ROT_SPIN_EDIT), EDITTYPE_FLOAT);
			MinRotSpin->SetLimits(0.f, 100.f, TRUE);
			MinRotSpin->SetScale(0.1f);
			MinRotSpin->SetValue(0.001f, FALSE);
			ReleaseISpinner(MinRotSpin);

			LoopToFrameSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_LOOP_TO_FRAME_SPIN));
			LoopToFrameSpin->LinkToEdit(GetDlgItem(HWnd, IDC_KF_EXPORTER_LOOP_TO_FRAME_SPIN_EDIT), EDITTYPE_INT);
			LoopToFrameSpin->SetLimits(0, 100, TRUE);
			LoopToFrameSpin->SetScale(1);
			LoopToFrameSpin->SetValue(0, FALSE);
			ReleaseISpinner(LoopToFrameSpin);

			SendMessage(GetDlgItem(HWnd, IDC_KF_EXPORTER_INTERPOLATION_VALUE), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T("None")));
			SendMessage(GetDlgItem(HWnd, IDC_KF_EXPORTER_INTERPOLATION_VALUE), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T("Orhonormalized")));
			SendMessage(GetDlgItem(HWnd, IDC_KF_EXPORTER_INTERPOLATION_VALUE), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T("Normalized")));

			ComboBox_SetCurSel(GetDlgItem(HWnd, IDC_KF_EXPORTER_INTERPOLATION_VALUE), 0);

			SendMessage(GetDlgItem(HWnd, IDC_KF_EXPORTER_PATHS), CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T("textures;..\\sharedtextures")));
			ComboBox_SetCurSel(GetDlgItem(HWnd, IDC_KF_EXPORTER_PATHS), 0);

			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_MP1), TRUE);

			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_USE_GLOBAL_ANIMATION), TRUE);
			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_ANIMATIONS), TRUE);
			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_MATERIALS), TRUE);
			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_FLOATING_VERTICES), TRUE);
			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_HIDDEN_OBJECTS), TRUE);
			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_SCALE), TRUE);
			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_RETAIN_HIERARCHIES), TRUE);
			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_USE_PIVOT), TRUE);
			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_SAVE_REFS), TRUE);
			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_GEOMETRY), TRUE);

			EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_LOOP_INTERP), FALSE);
			EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_LOOP_DEC_FRAME), FALSE);
			EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_MAINTAIN_MATRIX), FALSE);

			Edit_SetText(GetDlgItem(HWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), _T("_skin.skd"));
			EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), FALSE);

			//Start unsupporter stuff
			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_SAVE_REFS), FALSE);
			EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_SAVE_REFS), FALSE);

			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_USE_PIVOT), FALSE);
			EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_USE_PIVOT), FALSE);

			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_FLOATING_VERTICES), FALSE);
			EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_FLOATING_VERTICES), FALSE);
			
			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_CAMERAS), FALSE);
			EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_CAMERAS), FALSE);

			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_LIGHTS), FALSE);
			EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_LIGHTS), FALSE);

			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_ENVIRONMENT), FALSE);
			EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_ENVIRONMENT), FALSE);

			Button_SetCheck(GetDlgItem(HWnd, IDC_KF_EXPORTER_HELPERS), FALSE);
			EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_HELPERS), FALSE);
			//End unsuported stuff

			return TRUE;
		case WM_COMMAND:
			switch (HIWORD(WParam)) {
				case CBN_SELCHANGE:
				{
					if (LRESULT SelectedItemIndex = ComboBox_GetCurSel(GetDlgItem(HWnd, IDC_KF_EXPORTER_INTERPOLATION_VALUE));  SelectedItemIndex <= 0) {
						EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_MAINTAIN_MATRIX), FALSE);
					}
					else {
						EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_MAINTAIN_MATRIX), TRUE);
					}
					
					return TRUE;
				}
			}
			switch (LOWORD(WParam)) {
			case IDC_KF_EXPORTER_LOOP:
				if (IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_LOOP) == BST_CHECKED) {
					EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_LOOP_INTERP), TRUE);
					EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_LOOP_DEC_FRAME), TRUE);
				}
				else {
					EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_LOOP_INTERP), FALSE);
					EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_LOOP_DEC_FRAME), FALSE);
				}
				return TRUE;
			case IDC_KF_EXPORTER_SCALE:
				if (IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_SCALE) == BST_CHECKED) {
					ScaleValueSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_SCALE_SPIN));
					ScaleValueSpin->Enable(TRUE);
					ReleaseISpinner(ScaleValueSpin);
				}
				else {
					ScaleValueSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_SCALE_SPIN));
					ScaleValueSpin->Enable(FALSE);
					ReleaseISpinner(ScaleValueSpin);
				}
				return TRUE;
			case IDC_KF_EXPORTER_GEOMETRY:
				if (IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_GEOMETRY) == BST_CHECKED) {
					EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_SKINNING), TRUE);
					if (IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_SKINNING) == BST_CHECKED) {
						EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), TRUE);
					}
					else {
						EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), FALSE);
					}
				}
				else {
					EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_SKINNING), FALSE);
					EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), FALSE);
				}
				return TRUE;
			case IDC_KF_EXPORTER_SKINNING:
				if (IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_SKINNING) == BST_CHECKED) {
					EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), TRUE);
				}
				else {
					EnableWindow(GetDlgItem(HWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), FALSE);
				}
				return TRUE;
			case IDOK: {
				MPKFExporterOptions* Options = Exporter->GetExportOptions();
				Options->ExportGeometry = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_GEOMETRY) == BST_CHECKED;
				Options->ExportSkinning = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_SKINNING) == BST_CHECKED;
				Options->ExportAnimations = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_ANIMATIONS) == BST_CHECKED;
				Options->ExportMaterials = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_MATERIALS) == BST_CHECKED;
				Options->ExportCameras = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_CAMERAS) == BST_CHECKED;
				Options->ExportLights = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_LIGHTS) == BST_CHECKED;
				Options->ExportEnvironmets = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_ENVIRONMENT) == BST_CHECKED;
				Options->ExportHelpers = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_HELPERS) == BST_CHECKED;
				Options->Scale = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_SCALE) == BST_CHECKED;
				Options->CopyTexturesToExportPath = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_COPY_TEXTURES) == BST_CHECKED;
				Options->RetainHierarchies = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_RETAIN_HIERARCHIES) == BST_CHECKED;
				Options->UsePivotAsMeshCenter = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_USE_PIVOT) == BST_CHECKED;
				Options->SaveReferencesOnlyOnce = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_SAVE_REFS) == BST_CHECKED;
				Options->RemoveFloatingVertices = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_FLOATING_VERTICES) == BST_CHECKED;
				Options->RemoveHiddenObjects = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_HIDDEN_OBJECTS) == BST_CHECKED;
				Options->UsePivotAsMeshCenter = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_USE_PIVOT) == BST_CHECKED;
				Options->Game = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_MP1) == BST_CHECKED ? 0 : 1;
				ScaleValueSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_SCALE_SPIN));
				Options->ScaleValue = ScaleValueSpin->GetFVal();
				ReleaseISpinner(ScaleValueSpin);

				Options->UseGlobalAnimationRange = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_USE_GLOBAL_ANIMATION) == BST_CHECKED;
				Options->MaintainMatrixScaling = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_MAINTAIN_MATRIX) == BST_CHECKED;
				Options->LoopWhenFinished = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_LOOP) == BST_CHECKED;
				Options->LoopInterpolation = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_LOOP_INTERP) == BST_CHECKED;
				Options->DecEndFrameByOne = IsDlgButtonChecked(HWnd, IDC_KF_EXPORTER_LOOP_DEC_FRAME) == BST_CHECKED;

				SampleValueSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_SAMPLE_SPIN));
				Options->SampleRate = SampleValueSpin->GetIVal();
				ReleaseISpinner(SampleValueSpin);

				StartFrameSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_START_FRAME_SPIN));
				Options->StartFrame = StartFrameSpin->GetIVal();
				ReleaseISpinner(StartFrameSpin);

				EndFrameSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_END_FRAME_SPIN));
				Options->EndFrame = EndFrameSpin->GetIVal();
				ReleaseISpinner(EndFrameSpin);

				MinPosSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_MIN_POS_SPIN));
				Options->MinDeltaPosition = MinPosSpin->GetFVal();
				ReleaseISpinner(MinPosSpin);

				MinRotSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_MIN_ROT_SPIN));
				Options->MinDeltaRotation = MinRotSpin->GetFVal();
				ReleaseISpinner(MinRotSpin);

				LoopToFrameSpin = GetISpinner(GetDlgItem(HWnd, IDC_KF_EXPORTER_LOOP_TO_FRAME_SPIN));
				Options->LoopToFrame = LoopToFrameSpin->GetIVal();
				ReleaseISpinner(LoopToFrameSpin);

				if (LRESULT SelectedItemIndex = SendMessage(GetDlgItem(HWnd, IDC_KF_EXPORTER_INTERPOLATION_VALUE), CB_GETCURSEL, 0, 0);  SelectedItemIndex >= 0) {
					Options->FrameToFrameRotationInterpolation = SelectedItemIndex;
				}
				else {
					Options->FrameToFrameRotationInterpolation = 0;
				}
			
				LRESULT TexturePathStringLen = ComboBox_GetTextLength(GetDlgItem(HWnd, IDC_KF_EXPORTER_PATHS));
				TCHAR* ComboboxText = new TCHAR[TexturePathStringLen + 1];
				ComboBox_GetText(GetDlgItem(HWnd, IDC_KF_EXPORTER_PATHS), ComboboxText, TexturePathStringLen + 1);
				Options->Paths = TSTR(ComboboxText);

				if (!IsASCII(Options->Paths.data())) {
					MaxSDK::MaxMessageBox(HWnd, _T("Path contains non-ASCII characters, only english characters is allowed"), _T("Error"), MB_OK | MB_ICONERROR);
					break;
				}

				LRESULT SkinningPathStringLen = Edit_GetTextLength(GetDlgItem(HWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX));
				TCHAR* SkinningText = new TCHAR[SkinningPathStringLen + 1];
				ComboBox_GetText(GetDlgItem(HWnd, IDC_KF_EXPORTER_SKINNING_POSTFIX), SkinningText, SkinningPathStringLen + 1);
				Options->SkinningPostfix = TSTR(SkinningText);

				if (Options->SkinningPostfix.Length() == 0 || !IsASCII(Options->SkinningPostfix.data())) {
					MaxSDK::MaxMessageBox(HWnd, _T("Skin file postfix contains non-ASCII characters or empty, only english characters is allowed"), _T("Error"), MB_OK | MB_ICONERROR);
					break;
				}

				EndDialog(HWnd, 1);
				break;
			}
			case IDCANCEL:
				EndDialog(HWnd, 0);
				break;
			}
            return TRUE;
		default:
			return FALSE;
	}

	return TRUE;
}

int MPKFExporter::DoExport(const MCHAR *Name, ExpInterface *Ei, Interface *I, BOOL SuppressPrompts, DWORD Options)
{
	gMaxWndHandle = I->GetMAXHWnd();
	
	ExportOptions.ResetToDefault();

    if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_KF_EXPORTER_DIALOG), gMaxWndHandle, KFExportDlgProc, reinterpret_cast<LPARAM>(this))) {
		return IMPEXP_CANCEL;
	}
	
    GameScene = GetIGameInterface();

	IGameConversionManager* ConversionManager = GetConversionManager();
	ConversionManager->SetCoordSystem(IGameConversionManager::CoordSystem::IGAME_D3D);

	GameScene->InitialiseIGame(false);
	GameScene->SetStaticFrame(0);

	std::unique_ptr<MPKFScene> Scene = std::make_unique<MPKFScene>();
	for (int NodeIdx{ 0 }; NodeIdx < GameScene->GetTopLevelNodeCount(); ++NodeIdx)
	{
		if (!DoIterateSubNodes(GameScene->GetTopLevelNode(NodeIdx), Scene.get())) {
			return IMPEXP_FAIL;
		}
	}
	
	Scene->FixTransforms(ExportOptions.RetainHierarchies);

	TSTR FilePath, FileName, FileExt;
	SplitFilename(TSTR(Name), &FilePath, &FileName, &FileExt);

	MPMemoryWriter MemoryWriter;
	if (ExportOptions.ExportMaterials) {
		if (!ExporterExportKFMaterials(Scene.get(), FilePath, ExportOptions, MemoryWriter)) {
			return IMPEXP_FAIL;
		}
	}

	if (ExportOptions.ExportGeometry) {
		if (!ExporterExportKFMeshes(Scene.get(), ExportOptions, MemoryWriter)) {
			return IMPEXP_FAIL;
		}
	}

	if (ExportOptions.ExportSkinning) {
		MPMemoryWriter SkinMemoryWriter;

		if (!ExporterExportKFSkins(Scene.get(), ExportOptions, SkinMemoryWriter)) {
			return IMPEXP_FAIL;
		}

		TSTR SkinFile = FileName + ExportOptions.SkinningPostfix;

		FILE* OutputFile = _tfopen(SkinFile.data(), _T("wb"));

		fwrite(SkinMemoryWriter.GetData(), SkinMemoryWriter.GetSize(), 1, OutputFile);

		fclose(OutputFile);
	}

		
	FILE* OutputFile = _tfopen(Name, _T("wb"));

	fwrite(MemoryWriter.GetData(), MemoryWriter.GetSize(), 1, OutputFile);

	fclose(OutputFile);

	MaxSDK::MaxMessageBox(gMaxWndHandle, _T("Exported"), _T("Info"), MB_OK | MB_ICONINFORMATION);

	return IMPEXP_SUCCESS;
}

bool MPKFExporter::DoIterateSubNodes(IGameNode* Node, MPKFScene* Scene)
{
	assert(Node);

	if (!Node->IsTarget() && !Node->IsGroupOwner() && !(Node->IsNodeHidden() && ExportOptions.RemoveHiddenObjects)) {
		if (!DoExportNode(Node, Scene)) {
			return false;
		}
	}

	for (int NodeIdx{0}; NodeIdx < Node->GetChildCount(); ++NodeIdx) {
		const TCHAR* Name = Node->GetName();
		if (!DoIterateSubNodes(Node->GetNodeChild(NodeIdx), Scene)) {
			return false;
		}
	}
	
	return true;
}

bool MPKFExporter::DoExportNode(IGameNode* Node, MPKFScene* Scene) const
{
	assert(Node);
	
	IGameObject* GameObject = Node->GetIGameObject();
	auto T = GameObject->GetIGameType();

	switch (T)  // NOLINT(clang-diagnostic-switch-enum)
	{
	case IGameObject::IGAME_MESH: {
		if (ExportOptions.ExportGeometry || ExportOptions.ExportAnimations || ExportOptions.ExportSkinning) {
			MPKFMesh* Mesh;
			if (!ExporterCreateKFMesh(Node, GameObject, dynamic_cast<IGameMesh*>(GameObject), &Mesh)) {
				return false;
			}
			Scene->AddNode(Mesh);
		}
		break;
	}
	case IGameObject::IGAME_CAMERA:
	case IGameObject::IGAME_LIGHT:
	case IGameObject::IGAME_HELPER:
	case IGameObject::IGAME_BONE:
		break;
	default:
		SHOW_ERROR(MPKFEXPORTER_ERROR_UNSUPPORTED_OBJECTS, Node->GetName())
		Node->ReleaseIGameObject();
		return false;
	}

	Node->ReleaseIGameObject();

	return true;
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

	int SceneTicks = GameScene->GetSceneTicks();

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

	//Context->Animations.push_back(Animation);

	return true;
}

bool MPKFExporter::DoExportKFAnimation(IGameNode* Node, MPKFMeshExporterContext* Context, MPKFGlobalExporterContext* GlobalContext)
{
	/*for (MPKFAnimationExporterContext& Animation : Context->Animations) {

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
	}*/

	return true;
}
