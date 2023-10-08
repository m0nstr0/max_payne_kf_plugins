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
#include <MPKFType.h>

#define WRITE_ASCII_STRING(VariableName, MCharString, Writer)	\
	TSTR VariableName = TSTR(MCharString);						\
	if (!IsASCII(VariableName.data())) {						\
		return false;											\
	}															\
	CStr VariableName##Asc = VariableName.ToCStr();				\
	*Writer << MPString(VariableName##Asc.data(), VariableName##Asc.Length());	

#define MPKFEXPORTER_ERROR_NONE		0
#define MPKFEXPORTER_ERROR_NONE1	0
#define MPKFEXPORTER_ERROR_NONE2	0
#define MPKFEXPORTER_ERROR_NONE3	0
#define MPKFEXPORTER_ERROR_NONE4	0
#define MPKFEXPORTER_ERROR_NONE5	0
#define MPKFEXPORTER_ERROR_NONE6	0
#define MPKFEXPORTER_ERROR_NONE7	0
#define MPKFEXPORTER_ERROR_NONE8	0

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

	if (!DoExportMaterials(MemoryWriter, FilePath)) {
		return IMPEXP_FAIL;
	}

	if (!DoExportNodes(MemoryWriter)) {
		return IMPEXP_FAIL;
	}

	FILE* OutputFile = _tfopen(name, _T("wb"));

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

bool MPKFExporter::DoExportNodes(MPMemoryWriter& MemoryWriter)
{
	const int TopLevelNodesNum = pIgame->GetTopLevelNodeCount();

	for (int TopLevelNodeIndex{ 0 }; TopLevelNodeIndex < TopLevelNodesNum; TopLevelNodeIndex++)
	{
		IGameNode* Node = pIgame->GetTopLevelNode(TopLevelNodeIndex);

		IGameObject* GameObject = Node->GetIGameObject();
		IGameObject::ObjectTypes ObjectType = GameObject->GetIGameType();

		if (ObjectType == IGameObject::IGAME_MESH) {
			IGameMesh* Mesh = (IGameMesh*)GameObject;
			std::unique_ptr<MPMemoryChunkWriter> MeshChunk{ MemoryWriter.CreateChunk(0x0C, MPKFTYPE_MESH_CHUNK_ID, 2) };
			if (!DoExportMesh(Node, Mesh, MeshChunk.get())) {
				Node->ReleaseIGameObject();
				return false;
			}
			MemoryWriter << MeshChunk.get();
			MeshChunk.reset();
		}

		Node->ReleaseIGameObject();

		//Node->GetChildCount();

		//! Access the n'th child node of the parent node
		/*!
		\param index The index to the child to retrieve
		\return IGameNode pointer to the child
		*/
		//Node->GetNodeChild(int index);

		//if (ExportOptions.RemoveHiddenObjects && Node->IsNodeHidden()) {
		//	continue;
		//}

		
		
		
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
	}

	return true;
}

bool MPKFExporter::DoExportNode(IGameNode* Node, MPMemoryChunkWriter* ChunkWriter)
{
	WRITE_ASCII_STRING(NodeName, Node->GetName(), ChunkWriter);

	IGameNode* ParentNode = Node->GetNodeParent();
	if (ParentNode) {
		WRITE_ASCII_STRING(ParentNodeName, ParentNode->GetName(), ChunkWriter);
	}
	else {
		*ChunkWriter << MPString(nullptr, 0);
	}

	GMatrix LocalMatrix = Node->GetLocalTM();
	Point4 Row1 = LocalMatrix.GetRow(0);
	Point4 Row2 = LocalMatrix.GetRow(1);
	Point4 Row3 = LocalMatrix.GetRow(2);
	Point4 Row4 = LocalMatrix.GetRow(3); 

	MPMatrix4x3 NodeMatrix(Row1.x, Row1.y, Row1.z, Row2.x, Row2.y, Row2.z, Row3.x, Row3.y, Row3.z, Row4.x, Row4.y, Row4.z);

	*ChunkWriter << NodeMatrix;

	if (ParentNode) {
		*ChunkWriter << true;
	}
	else {
		*ChunkWriter << false;
	}

	*ChunkWriter << MPString(nullptr, 0);

	return true;
}

bool MPKFExporter::DoExportMesh(IGameNode* Node, IGameMesh* Mesh, MPMemoryChunkWriter* ChunkWriter)
{
	Mesh->SetCreateOptimizedNormalList();
	if (!Mesh->InitializeData()) {
		return false;
	}

	std::unique_ptr<MPMemoryChunkWriter> NodeChunk{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_NODE_SUB_CHUNK_ID, 1) };
	if (!DoExportNode(Node, NodeChunk.get())) {
		return false;
	}
	*ChunkWriter << NodeChunk.get();
	NodeChunk.reset();

	//Geometry
	std::unique_ptr<MPMemoryChunkWriter> GeometryChunk{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_GEOMETRY_SUB_CHUNK_ID, 1) };
	std::vector<MPVector3> LocalNormals;
	std::vector<MPVector2> LocalTexVertices;
	std::vector<MPVector3> LocalVertices;
	std::vector<uint16_t> LocalIndices;
	std::map<uint16_t, std::map<uint16_t, uint16_t>> MaxVertexToLocal;

	std::vector<MPVector3> MeshVertices;
	std::vector<MPVector2> MeshTexVertices;
	std::vector<MPVector3> MeshNormals;
	std::vector<uint16_t> MeshIndices;
	std::vector<uint32_t> MeshVerticesPerPrimitive;
	std::vector<uint32_t> MeshIndicesPerPrimitive;
	std::vector<IGameMaterial*> MaterialPerPrimitive;

	Tab<int> ActiveMatIDs = Mesh->GetActiveMatIDs();

	for (int ActiveMatID = 0; ActiveMatID < ActiveMatIDs.Count(); ActiveMatID++)
	{
		LocalNormals.clear();
		LocalVertices.clear();
		LocalIndices.clear();
		LocalTexVertices.clear();
		MaxVertexToLocal.clear();

		Tab<FaceEx*> Faces = Mesh->GetFacesFromMatID(ActiveMatIDs[ActiveMatID]);
		for (int FaceID = 0; FaceID < Faces.Count(); FaceID++)
		{
			FaceEx* Face = Faces[FaceID];
		
			if (FaceID == 0) {
				MaterialPerPrimitive.push_back(Mesh->GetMaterialFromFace(Face->meshFaceIndex));
			}

			for (uint16_t i = 0; i < 3; i++) {
				if (MaxVertexToLocal.contains(Face->vert[i]) && MaxVertexToLocal[Face->vert[i]].contains(Face->norm[i])) {
					LocalIndices.push_back(MaxVertexToLocal[Face->vert[i]][Face->norm[i]]);
					continue;
				}

				MaxVertexToLocal[Face->vert[i]][Face->norm[i]] = LocalVertices.size();

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

				LocalIndices.push_back(MaxVertexToLocal[Face->vert[i]][Face->norm[i]]);
			}
		}

		for (MPVector3& Vertex : LocalVertices) {
			MeshVertices.push_back(Vertex);
		}

		for (MPVector3& Normal : LocalNormals) {
			MeshNormals.push_back(Normal);
		}

		for (MPVector2& TexVertex : LocalTexVertices) {
			MeshTexVertices.push_back(TexVertex);
		}

		for (uint16_t Index : LocalIndices) {
			MeshIndices.push_back(Index);
		}

		MeshVerticesPerPrimitive.push_back(LocalVertices.size());

		MeshIndicesPerPrimitive.push_back(LocalIndices.size());
	}

	*GeometryChunk << static_cast<uint32_t>(MeshVertices.size());

	for (const MPVector3& Vertex : MeshVertices) {
		GeometryChunk->Write(&Vertex.X, sizeof(float));
		GeometryChunk->Write(&Vertex.Y, sizeof(float));
		GeometryChunk->Write(&Vertex.Z, sizeof(float));
	}

	for (const MPVector3& Vertex : MeshNormals) {
		GeometryChunk->Write(&Vertex.X, sizeof(float));
		GeometryChunk->Write(&Vertex.Y, sizeof(float));
		GeometryChunk->Write(&Vertex.Z, sizeof(float));
	}

	*GeometryChunk << static_cast<uint32_t>(MeshVerticesPerPrimitive.size());

	for (uint32_t NumberVertices : MeshVerticesPerPrimitive) {
		*GeometryChunk << NumberVertices;
	}

	*ChunkWriter << GeometryChunk.get();
	GeometryChunk.reset();

	//Polygons
	std::unique_ptr<MPMemoryChunkWriter> PolygonsChunk{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_POLYGONS_SUB_CHUNK_ID, 1) };

	*PolygonsChunk << static_cast<uint32_t>(MeshIndices.size());

	for (const uint16_t Index : MeshIndices) {
		PolygonsChunk->Write(&Index, sizeof(uint16_t));
	}
	
	*PolygonsChunk << static_cast<uint32_t>(MeshIndicesPerPrimitive.size());

	for (const uint16_t NumIndices : MeshIndicesPerPrimitive) {
		*PolygonsChunk << static_cast<uint32_t>(NumIndices / 3);
	}

	*ChunkWriter << PolygonsChunk.get();
	PolygonsChunk.reset();

	//TODO: Skip?
	//Polygon Materials
	std::unique_ptr<MPMemoryChunkWriter> PolygonMaterialsChunk{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_POLYGON_MATERIAL_SUB_CHUNK_ID, 1) };

	*PolygonMaterialsChunk << static_cast<int32_t>(MaterialPerPrimitive.size());
	
	for (IGameMaterial* Material : MaterialPerPrimitive) {
		WRITE_ASCII_STRING(MaterialName, Material->GetMaterialName(), PolygonMaterialsChunk);
	}

	*ChunkWriter << PolygonMaterialsChunk.get();
	PolygonMaterialsChunk.reset();

	//Polygon UVW
	std::unique_ptr<MPMemoryChunkWriter> UVChunk{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_UV_MAPPING_SUB_CHUNK_ID, 1) };

	*UVChunk << static_cast<uint32_t>(0);
	
	*UVChunk << static_cast<uint32_t>(MeshTexVertices.size());

	for (MPVector2& Vertex : MeshTexVertices) {
		float Z = 0.f;
		UVChunk->Write(&Vertex.X, sizeof(float));
		UVChunk->Write(&Vertex.Y, sizeof(float));
		UVChunk->Write(&Z, sizeof(float));
	}

	*UVChunk << static_cast<uint32_t>(MeshVerticesPerPrimitive.size());

	for (uint32_t NumberVertices : MeshVerticesPerPrimitive) {
		*UVChunk << NumberVertices;
	}

	*ChunkWriter << UVChunk.get();
	UVChunk.reset();

	return true;
}

bool MPKFExporter::DoExportMaterials(MPMemoryWriter& MemoryWriter, const TSTR& CopyDirTo)
{
	if (!ExportOptions.ExportMaterials) {
		return true;
	}

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
				if (!DoExportMaterial(Material->GetSubMaterial(SumMaterialIndex), CopyDirTo, MaterialChunk.get())) {
					return false;
				}
			}
		} else {
			if (!DoExportMaterial(Material, CopyDirTo, MaterialChunk.get())) {
				return false;
			}
		}
		*MaterialListChunk << MaterialChunk.get();
	}

	MemoryWriter << MaterialListChunk.get();

	return true;
}

bool MPKFExporter::DoExportTexture(Texmap* Texture, const TSTR& CopyDirTo, MPMemoryChunkWriter* ChunkWriter)
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

bool MPKFExporter::DoExportMaterial(IGameMaterial* Material, const TSTR& CopyDirTo, MPMemoryChunkWriter* ChunkWriter)
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
		if (!DoExportTexture(KFMaterial->GetDiffuseTexture(), CopyDirTo, ChunkWriter)) {
			return false;
		}
	}

	*ChunkWriter << KFMaterial->HasReflectionTexture();
	if (KFMaterial->HasReflectionTexture()) {
		if (!DoExportTexture(KFMaterial->GetReflectionTexture(), CopyDirTo, ChunkWriter)) {
			return false;
		}
	}

	if (ExportOptions.Game == 0) {
		*ChunkWriter << KFMaterial->HasBumpTexture();
		if (KFMaterial->HasBumpTexture()) {
			if (!DoExportTexture(KFMaterial->GetBumpTexture(), CopyDirTo, ChunkWriter)) {
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
			if (!DoExportTexture(KFMaterial->GetAlphaTexture(), CopyDirTo, ChunkWriter)) {
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
			if (!DoExportTexture(KFMaterial->GetMaskTexture(), CopyDirTo, ChunkWriter)) {
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