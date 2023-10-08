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

MPKFExporter::MPKFExporter(): 
	ExportOptions()
{

}

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
					ScaleValueSpin->Enable(FALSE);
					ReleaseISpinner(ScaleValueSpin);
				}
				else {
					ScaleValueSpin = GetISpinner(GetDlgItem(hWnd, IDC_KF_EXPORTER_SCALE_SPIN));
					ScaleValueSpin->Enable(TRUE);
					ReleaseISpinner(ScaleValueSpin);
				}
				return TRUE;
			case IDC_KF_EXPORTER_GEOMETRY:
				if (IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_GEOMETRY) == BST_CHECKED) {
					EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING), TRUE);
				}
				else {
					EnableWindow(GetDlgItem(hWnd, IDC_KF_EXPORTER_SKINNING), FALSE);
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
				Options->ExportHelpers = IsDlgButtonChecked(hWnd, IDC_KF_EXPORTER_HELPERS) == BST_CHECKED;
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

				//ComboBox_GetCurSel
				
				LRESULT TexturePathStringLen = ComboBox_GetTextLength(GetDlgItem(hWnd, IDC_KF_EXPORTER_PATHS));
				TCHAR* ComboboxText = new TCHAR[TexturePathStringLen + 1];
				ComboBox_GetText(GetDlgItem(hWnd, IDC_KF_EXPORTER_PATHS), ComboboxText, TexturePathStringLen + 1);
				Options->Paths = TSTR(ComboboxText);

				if (!IsASCII(Options->Paths.data())) {
					MaxSDK::MaxMessageBox(hWnd, _T("Path contains non-ASCII characters, only english characters is allowed"), _T("Error"), MB_OK | MB_ICONERROR);
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

    if (!DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_KF_EXPORTER_DIALOG), MaxHWND, KFExportDlgProc, (LPARAM)this)) {
		return IMPEXP_CANCEL;
	}

    pIgame = GetIGameInterface();

	IGameConversionManager* cm = GetConversionManager();
	cm->SetCoordSystem(IGameConversionManager::CoordSystem::IGAME_D3D);

	pIgame->InitialiseIGame(false);
	pIgame->SetStaticFrame(0);

	TSTR FilePath, FileName, FileExt;
	SplitFilename(TSTR(name), &FilePath, &FileName, &FileExt);

	MPMemoryWriter MemoryWriter;

	DoExportMaterials(MemoryWriter);

	FILE* OutputFile = _tfopen(name, _T("w"));

	fwrite(MemoryWriter.GetData(), MemoryWriter.GetSize(), 1, OutputFile);

	fclose(OutputFile);

	return IMPEXP_SUCCESS;

    //int TopLevelNodesNum = pIgame->GetTopLevelNodeCount();

	//for (int TopLevelNodeIndex{ 0 }; TopLevelNodeIndex < TopLevelNodesNum; TopLevelNodeIndex++)
	//{
	//	IGameNode* Node = pIgame->GetTopLevelNode(TopLevelNodeIndex);
		//MCHAR* NodeName = Node->GetName();
	//	IGameObject* obj = Node->GetIGameObject();
	//}
        //
       // 
       // 

       // switch(obj->GetIGameType())
		//{
           // case IGameObject::IGAME_MESH: {
               // IGameMesh * gM = (IGameMesh*)obj;
				//gM->SetCreateOptimizedNormalList();
					//if(gM->InitializeData())
					//{
						//CreateXMLNode(pXMLDoc,parent,_T("Mesh"),&geomData);
						//if(splitFile)
						//{
						//	TSTR filename;
						//	MakeSplitFilename(child,filename);
							
						//	AddXMLAttribute(geomData, _T("Include"),filename.data());
						//	CreateXMLNode(pSubDocMesh,pSubMesh,_T("Mesh"),&subMeshNode);
						//	AddXMLAttribute(subMeshNode,_T("Node"),child->GetName());
						//	geomData = subMeshNode;

						//}
						//DumpMesh(gM,geomData);
					//}
           // }
        //}

       // Node->ReleaseIGameObject();
        //MCHAR * 	GetName ()=0
        /*
        virtual GMatrix 	GetWorldTM (TimeValue t=TIME_NegInfinity)=0
     	Get World TM.
 
        virtual GMatrix 	GetLocalTM (TimeValue t=TIME_NegInfinity)=0
 	    Get Local TM.
 
        virtual GMatrix 	GetObjectTM (TimeValue t=TIME_NegInfinity)=0

        virtual IGameNode * 	GetNodeParent ()=0
        Get the nodes parent.
    
        virtual int 	GetChildCount ()=0
            Get the number of direct children to the parent.
        
        virtual IGameNode * 	GetNodeChild (int index)=0
            Access the n'th child node of the parent node.
            virtual int 	GetMaterialIndex ()=0
            Get the material index.
        
        virtual IGameMaterial * 	GetNodeMaterial ()=0
            Get the material.
        
        virtual IPoint3 & 	GetWireframeColor ()=0
            Get the wireframe color.
        
        virtual bool 	IsTarget ()=0
            Check if a Target Node

        
        virtual bool 	IsGroupOwner ()=0
            Check if a Group Head.
        
        virtual bool 	IsNodeHidden ()=0
            Check if the node hidden.

            irtual IGameObject * 	GetIGameObject ()=0
            Get the actual object.
        
        virtual void 	ReleaseIGameObject ()=0
        Release the IGameObject obtained from GetIGameObject.
        */

    //}
    
    //IGameMaterial * 	GetIGameMaterial (Mtl *mat)=0
    //virtual IGameTextureMap * 	GetIGameTextureMap (Texmap *texMap)=0
 
    //pIgame->ReleaseIGame();
 	
    return IMPEXP_SUCCESS;
}

bool MPKFExporter::DoExportMesh(IGameMesh* Mesh)
{
	return false;
}

bool MPKFExporter::DoExportMaterials(MPMemoryWriter& MemoryWriter)
{
	if (ExportOptions.ExportMaterials)
	{
		std::unique_ptr<MPMemoryChunkWriter> MaterialListChunk{ MemoryWriter.CreateChunk(0x0C, MPKFTYPE_MATERIAL_LIST_CHUNK_ID, 0) };

		const int NumberOfRootMaterials = pIgame->GetRootMaterialCount();

		int32_t TotalNumberOfMaterials = 0;
		for (int MaterialIndex = 0; MaterialIndex < NumberOfRootMaterials; MaterialIndex++) {
			IGameMaterial* Material = pIgame->GetRootMaterial(MaterialIndex);
			if (Material->IsSubObjType()) {
				TotalNumberOfMaterials += Material->GetSubMaterialCount();
			}
			else {
				TotalNumberOfMaterials += 1;
			}
		}

		CStr PathsAsc = ExportOptions.Paths.ToCStr();
		*MaterialListChunk << MPString(PathsAsc.data(), PathsAsc.Length());

		*MaterialListChunk << TotalNumberOfMaterials;

		for (int MaterialIndex = 0; MaterialIndex < NumberOfRootMaterials; MaterialIndex++) {
			std::unique_ptr<MPMemoryChunkWriter> MaterialChunk{ MemoryWriter.CreateChunk(0x0C, MPKFTYPE_MATERIAL_SUB_CHUNK_ID, ExportOptions.Game == 0 ? 1 : 0) };
			IGameMaterial* Material = pIgame->GetRootMaterial(MaterialIndex);
			if (Material->IsSubObjType()) {
				for (int SumMaterialIndex = 0; SumMaterialIndex < Material->GetSubMaterialCount(); SumMaterialIndex++) {
					if (!DoExportMaterial(Material->GetSubMaterial(SumMaterialIndex), MaterialChunk.get())) {
						return false;
					}
				}
			} else {
				if (!DoExportMaterial(Material, MaterialChunk.get())) {
					return false;
				}
			}
			*MaterialListChunk << MaterialChunk.get();
		}

		MemoryWriter << MaterialListChunk.get();
	}

	return true;
}

bool MPKFExporter::DoExportTexture(Texmap* Texture, MPMemoryChunkWriter* ChunkWriter)
{
	if (!Texture || Texture->ClassID() != MPKFTEXTURE_CLASS_ID) {
		return false;
	}

	std::unique_ptr<MPMemoryChunkWriter> TexureChunkWriter{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_TEXTURE_SUB_CHUNK_ID, 1)};
	IMPKFTexture* KFTexture = dynamic_cast<IMPKFTexture*>(Texture);

	const int32_t NumberOfTextures = KFTexture->GetTexturesCount();
	
	if (NumberOfTextures <= 0) {
		return false;
	}

	if (!IsASCII(KFTexture->GetTextureName())) {
		return false;
	}

	TSTR TextureName = TSTR(KFTexture->GetTextureName());
	CStr TextureNameAsc = TextureName.ToCStr();
	*TexureChunkWriter << MPString(TextureNameAsc.data(), TextureNameAsc.Length());

	if (KFTexture->IsMipMapsAuto()) {
		*TexureChunkWriter << (int32_t)(0);
	} else {
		*TexureChunkWriter << KFTexture->GetMipMapsNum();
	}
		
	*TexureChunkWriter << static_cast<int32_t>(KFTexture->GetFiltering());
	*TexureChunkWriter << NumberOfTextures;

	for (int32_t TextureIndex = 0; TextureIndex != NumberOfTextures; TextureIndex++) {
		const MCHAR* TextureFileName = KFTexture->GetTextureFileName(TextureIndex);

		if (TextureFileName == nullptr) {
			return false;
		}

		if (!IsASCII(TextureFileName)) {
			return false;
		}

		TSTR FilePath, FileName, FileExt;
		SplitFilename(TSTR(TextureFileName), &FilePath, &FileName, &FileExt);
		TSTR FileNameWithExt(FileName);
		FileNameWithExt.Append(FileExt);
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

bool MPKFExporter::DoExportMaterial(IGameMaterial* Material, MPMemoryChunkWriter* ChunkWriter)
{
	Mtl* MaxMaterial = Material->GetMaxMaterial();
	
	if (MaxMaterial->ClassID() != MPKFMATERIAL_CLASS_ID) {
		return false;
	}

	if (!IsASCII(Material->GetMaterialName())) {
		return false;
	}

	IMPKFMaterial* KFMaterial = dynamic_cast<IMPKFMaterial*>(MaxMaterial);

	if (!KFMaterial->IsAllTexturesCorrect()) {
		return false;
	}

	TSTR MaterialName = TSTR(Material->GetMaterialName());
	CStr MaterialNameAsc = MaterialName.ToCStr();
	*ChunkWriter << MPString(MaterialNameAsc.data(), MaterialNameAsc.Length());

	*ChunkWriter << KFMaterial->IsTwoSided();
	*ChunkWriter << KFMaterial->IsFogging();
	*ChunkWriter << false; //*ChunkWriter << KFMaterial->is_diffuse_combined();
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
		if (!DoExportTexture(KFMaterial->GetDiffuseTexture(), ChunkWriter)) {
			return false;
		}
	}

	*ChunkWriter << KFMaterial->HasReflectionTexture();
	if (KFMaterial->HasReflectionTexture()) {
		if (!DoExportTexture(KFMaterial->GetReflectionTexture(), ChunkWriter)) {
			return false;
		}
	}

	if (ExportOptions.Game == 0) {
		*ChunkWriter << KFMaterial->HasBumpTexture();
		if (KFMaterial->HasBumpTexture()) {
			if (!DoExportTexture(KFMaterial->GetBumpTexture(), ChunkWriter)) {
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
			if (!DoExportTexture(KFMaterial->GetAlphaTexture(), ChunkWriter)) {
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
			if (!DoExportTexture(KFMaterial->GetMaskTexture(), ChunkWriter)) {
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
		*ChunkWriter << KFMaterial->GetAlphaReferenceValue();
	}
	
	return true;
}

/*
	CComPtr <IXMLDOMNode> vertData,faceData;
	CComPtr <IXMLDOMNode> node;
	TSTR buf;

	pXMLDoc->createNode(CComVariant(NODE_ELEMENT), CComBSTR(_T("Faces")), NULL, &faceData);
	int numFaces = gm->GetNumberOfFaces();
	buf.printf(_T("%d"),numFaces);
	AddXMLAttribute(faceData,_T("Count"),buf.data());

	// Create 'Vertices' element to declare vertex positions
	CreateXMLNode(pXMLDoc,geomData,_T("Vertices"),&vertData);
	int numVerts = gm->GetNumberOfVerts();
	buf.printf(_T("%d"),numVerts);
	AddXMLAttribute(vertData,_T("Count"),buf.data());
	
	// WARNING: the order of creation of the text nodes is important to match
	// the schema. First vertices, then normals, etc

	// Create 'vertices' element of 'Faces' element to declare vertex indices
	CComPtr<IXMLDOMNode> vertices;
	CreateXMLNode(pXMLDoc,faceData,_T("FaceVertices"),&vertices);
	

	// dump Vertices
	for(int i = 0;i<numVerts;i++)
	{
		Point3 v; 
		if(gm->GetVertex(i,v, (exportObjectSpace != 0)))
			{
			buf.printf(_T("%f %f %f "),v.x,v.y,v.z);
			AddXMLText(pXMLDoc,vertData,buf.data());
		}
		}
			
	// TODO: Export Vertex selections
	// TODO: Export Vertex hide

	CComPtr<IXMLDOMNode> normals;
	if(exportNormals && !exportNormalsPerFace && gm->GetNumberOfNormals() > 0)
	{
		// dump Normals
		CComPtr <IXMLDOMNode> normData;
		int numNorms = gm->GetNumberOfNormals();
			CreateXMLNode(pXMLDoc,geomData,_T("Normals"),&normData);
			buf.printf(_T("%d"),numNorms);
			AddXMLAttribute(normData,_T("Count"),buf.data());

		// Create 'normals' element of 'Faces' element to declare normal indices
		CreateXMLNode(pXMLDoc,faceData,_T("FaceNormals"),&normals);
		
		for(int i = 0;i<numNorms;i++)
		{
			Point3 n;
			if(gm->GetNormal(i,n, (exportObjectSpace != 0)))
			{
				buf.printf(_T("%f %f %f "),n.x,n.y,n.z);
				AddXMLText(pXMLDoc,normData,buf.data());
			}
		}
	}

	// TODO: Export Vertex weights
	// TODO: Export soft selection

	Tab<int> matidTab = gm->GetActiveMatIDs();
	CComPtr<IXMLDOMNode> matids;

	if (matidTab.Count() > 0)
		{
		CreateXMLNode(pXMLDoc,faceData,_T("MaterialIDs"),&matids);
	}

	Tab <DWORD> smgrps = gm->GetActiveSmgrps();
	CComPtr<IXMLDOMNode> smgroups;
	if( smgrps.Count() > 0)
		CreateXMLNode(pXMLDoc,faceData,_T("SmoothingGroups"),&smgroups);

	CComPtr<IXMLDOMNode> edges;
	CreateXMLNode(pXMLDoc,faceData,_T("EdgeVisibility"),&edges);

	// dump Face data
	geomData->appendChild(faceData,NULL);
	for(int n=0;n<numFaces;n++)
	{
		FaceEx* f = gm->GetFace(n);

		if(vertices != NULL) {
			buf.printf(_T(" %d %d %d"), f->vert[0], f->vert[1], f->vert[2]);
			AddXMLText(pXMLDoc,vertices,buf.data());
		}

		if(normals != NULL) {
			buf.printf(_T(" %d %d %d"), f->norm[0], f->norm[1], f->norm[2]);
			AddXMLText(pXMLDoc,normals,buf.data());
			}

		if(smgroups != NULL) {
			buf.printf(_T(" %u"),(unsigned int)f->smGrp);
			AddXMLText(pXMLDoc,smgroups,buf.data());
		}

		if(matids != NULL) {
			buf.printf(_T(" %d"), f->matID);
			AddXMLText(pXMLDoc,matids,buf.data());
		}
		
		if(edges != NULL) {
			buf.printf(_T(" %d %d %d"), f->edgeVis[0], f->edgeVis[1], f->edgeVis[2] );
			AddXMLText(pXMLDoc,edges,buf.data());
		}
        



		// TODO: Export Face selection data
		// TODO: Export Face hide
		// TODO: Export edge selection
		
	}

	if(exportMappingChannel)
	{
		Tab<int> mapNums = gm->GetActiveMapChannelNum();
		int mapCount = mapNums.Count();

		if( mapCount > 0)
		{
		TSTR data;
		CComPtr <IXMLDOMNode> channelNode;
		CreateXMLNode(pXMLDoc,geomData,_T("MapChannels"),&channelNode);
		buf.printf(_T("%d"),mapCount);
		AddXMLAttribute(channelNode,_T("Count"),buf.data());

		for(int i=0;i < mapCount;i++)
		{
			CComPtr <IXMLDOMNode> channelItem, mvertData,vert,mfaceData,face;
			CreateXMLNode(pXMLDoc,channelNode,_T("MapChannel"),&channelItem);
			buf.printf(_T("%d"),mapNums[i]);
				AddXMLAttribute(channelItem,_T("ID"),buf.data());
				AddXMLAttribute(channelItem,_T("Type"),_T("Texture"));
			// TODO: Implement name attribute for channel

			CreateXMLNode(pXMLDoc,channelItem,_T("MapVertices"),&mvertData);
			int vCount = gm->GetNumberOfMapVerts(mapNums[i]);
			buf.printf(_T("%d"),vCount);
			AddXMLAttribute(mvertData,_T("Count"),buf.data());
			// TODO: Set proper map vertices dimension
//			int vDim = gm->GetDimensionOfMapVerts(mapNums[i]);
//			buf.printf("%d",vDim);
			AddXMLAttribute(mvertData,_T("Dimension"),_T("3"));

			for(int j=0;j<vCount;j++)
			{
				Point3 v;
				if(gm->GetMapVertex(mapNums[i],j,v))
				{
					data.printf(_T("%f %f %f"),v.x,v.y,v.z);
					AddXMLText(pXMLDoc,mvertData,data.data());
				}
		
			}

			CreateXMLNode(pXMLDoc,channelItem,_T("MapFaces"),&mfaceData);
			int fCount = gm->GetNumberOfFaces();
			buf.printf(_T("%d"),fCount);
			AddXMLAttribute(mfaceData,_T("Count"),buf.data());

			for(int k=0;k<fCount;k++)
			{
				DWORD  v[3];
				gm->GetMapFaceIndex(mapNums[i],k,v);
				data.printf(_T("%d %d %d"),v[0],v[1],v[2]);
				AddXMLText(pXMLDoc,mfaceData,data.data());
				}
			}
		}

	}

#if 0
//test code
	Tab<int> matids;

	matids = gm->GetActiveMatIDs();

	for(i=0;i<matids.Count();i++)
	{
		Tab<FaceEx*> faces;

		faces = gm->GetFacesFromMatID(matids[i]);

		for(int k=0; k<faces.Count();k++)
		{
			IGameMaterial * faceMat = gm->GetMaterialFromFace(faces[k]);
//			TSTR name(faceMat->GetMaterialName());
		}
		for(k=0;k<gm->GetNumberOfFaces();k++)
		{
			IGameMaterial * faceMat = gm->GetMaterialFromFace(k);
		}
		
	}

	Tab <DWORD> smgrps = gm->GetActiveSmgrps();
#endif

*/