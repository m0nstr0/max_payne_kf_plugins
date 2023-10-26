#include "MPKFMesh.h"
#include <map>
#include <memory>
#include <vector>
#include "IMPKFMaterial.h"
#include "IMPKFTexture.h"

// Create Mesh
bool ExporterCreateKFMesh(IGameNode* GameNode, IGameObject* GameObject, IGameMesh* GameMesh, MPKFMesh** OutMesh)
{
	assert(GameNode && GameObject && GameMesh);
	
	GameMesh->SetCreateOptimizedNormalList();
	if (!GameMesh->InitializeData()) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_MESH_TRIANGULATE, GameNode->GetName())
		return false;
	}

	IS_ASCII_STRING(GameNode->GetName())
	
	std::unique_ptr<MPKFMesh> Mesh{std::make_unique<MPKFMesh>(GameNode)};
	
	Mesh->OriginalMeshNumVertices = GameMesh->GetNumberOfVerts();

	std::map<IGameMaterial*, std::map<DWORD, std::map<DWORD, std::map<DWORD, uint32_t>>>> SubMeshLocalIndices;

	const Tab<int> MaterialIDs = GameMesh->GetActiveMatIDs();
	for (int MaterialId{0}; MaterialId < MaterialIDs.Count(); ++MaterialId)
	{
		const Tab<FaceEx*> Faces = GameMesh->GetFacesFromMatID(MaterialIDs[MaterialId]);
		for (int FaceID{0}; FaceID < Faces.Count(); ++FaceID)
		{
			FaceEx* Face = Faces[FaceID];
			IGameMaterial* Material = GameMesh->GetMaterialFromFace(Face);

			if (Material == nullptr) {
				SHOW_ERROR(MPKFEXPORTER_ERROR_OBJECTS_WITHOUT_MATERIAL, GameNode->GetName())
				return false;
			}

			MPKFSubMesh* SubMesh = Mesh->FindOrCreateSubMeshByMaterial(Material);

			for (unsigned int CornerId{0}; CornerId < 3; ++CornerId)
			{
				if (SubMeshLocalIndices.contains(Material)) {
					if (SubMeshLocalIndices[Material].contains(Face->vert[CornerId])) {
						if (SubMeshLocalIndices[Material][Face->vert[CornerId]].contains(Face->norm[CornerId])) {
							if (SubMeshLocalIndices[Material][Face->vert[CornerId]][Face->norm[CornerId]].contains(Face->texCoord[CornerId])) {
								SubMesh->Indices.push_back(SubMeshLocalIndices[Material][Face->vert[CornerId]][Face->norm[CornerId]][Face->texCoord[CornerId]]);
								continue;
							}
						}
					}
				}

				const uint32_t Index = SubMesh->Vertices.size();

				SubMeshLocalIndices[Material][Face->vert[CornerId]][Face->norm[CornerId]][Face->texCoord[CornerId]] = Index;
				
				SubMesh->Indices.push_back(Index);

				const Point3 Vertex = GameMesh->GetVertex(static_cast<int>(Face->vert[CornerId]), true);
				SubMesh->Vertices.emplace_back(Vertex.x, Vertex.y, Vertex.z);

				const Point3 Normal = GameMesh->GetNormal(static_cast<int>(Face->norm[CornerId]), true);
				SubMesh->Normals.emplace_back(Normal.x, Normal.y, Normal.z);

				const Point2 TexVertex = GameMesh->GetTexVertex(static_cast<int>(Face->texCoord[CornerId]));
				SubMesh->UVs.emplace_back(TexVertex.x, TexVertex.y, 0.f);

				SubMesh->LocalToOrigVertexId[Index] = static_cast<int>(Face->vert[CornerId]);
			}
		}
	}

	if (IGameSkin* GameSkin = GameObject->GetIGameSkin(); GameSkin != nullptr) {
		MPKFSkin* Skin{nullptr};
		if (!ExporterCreateKFSkin(GameSkin, Mesh.get(), &Skin)) {
			return false;
		}
		Mesh->Skin.reset(Skin);
	}
	
	*OutMesh = Mesh.release();
	return true;
}

// Export Mesh
bool ExporterExportKFUVWMapping(const MPKFMesh* Mesh, const MPKFExporterOptions& ExportOptions, MPMemoryChunkWriter* ChunkWriter)
{
	assert(Mesh);
	
	*ChunkWriter << static_cast<uint32_t>(0);

	uint32_t NumVertices{ 0 };
	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		NumVertices += SubMesh->Vertices.size();
	}

	*ChunkWriter << static_cast<uint32_t>(NumVertices);

	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		for (const MPVector3& UV : SubMesh->UVs) {
			ChunkWriter->Write(&UV.X, sizeof(float));
			ChunkWriter->Write(&UV.Y, sizeof(float));
			ChunkWriter->Write(&UV.Z, sizeof(float));
		}
	}

	*ChunkWriter << static_cast<uint32_t>(Mesh->SubMeshes.size());

	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		*ChunkWriter << static_cast<uint32_t>(SubMesh->Vertices.size());
	}

	return true;
}

bool ExporterExportKFPolygonMaterials(const MPKFMesh* Mesh, const MPKFExporterOptions& ExportOptions, MPMemoryChunkWriter* ChunkWriter)
{
	assert(Mesh);
	
	*ChunkWriter << static_cast<int32_t>(Mesh->SubMeshes.size());

	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		assert(SubMesh->Material);
		WRITE_ASCII_STRING(MaterialName, SubMesh->Material->GetMaterialName(), ChunkWriter)
	}

	return true;
}

bool ExporterExportKFGeometry(const MPKFMesh* Mesh, const MPKFExporterOptions& ExportOptions, MPMemoryChunkWriter* ChunkWriter)
{
	assert(Mesh);

	uint32_t NumVertices{ 0 };
	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		NumVertices += SubMesh->Vertices.size();
	}
	
	*ChunkWriter << static_cast<uint32_t>(NumVertices);

	float Scale = 1.f;

	if (ExportOptions.Scale) {
		Scale = ExportOptions.ScaleValue;
	}

	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {

		for (const MPVector3& Vertex : SubMesh->Vertices) {
			MPVector3 Scaled(Vertex);
			Scaled.ScaleBy(Scale);

			ChunkWriter->Write(&Scaled.X, sizeof(float));
			ChunkWriter->Write(&Scaled.Y, sizeof(float));
			ChunkWriter->Write(&Scaled.Z, sizeof(float));
		}
	}

	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		for (const MPVector3& Vertex : SubMesh->Normals) {
			ChunkWriter->Write(&Vertex.X, sizeof(float));
			ChunkWriter->Write(&Vertex.Y, sizeof(float));
			ChunkWriter->Write(&Vertex.Z, sizeof(float));
		}
	}
	
	*ChunkWriter << static_cast<uint32_t>(Mesh->SubMeshes.size());

	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		*ChunkWriter << static_cast<uint32_t>(SubMesh->Vertices.size());
	}

	return true;
}

bool ExporterExportKFPolygons(const MPKFMesh* Mesh, const MPKFExporterOptions& ExportOptions, MPMemoryChunkWriter* ChunkWriter)
{
	assert(Mesh);

	uint32_t NumIndices{0};
	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		NumIndices += SubMesh->Indices.size();
	}
	
	*ChunkWriter << static_cast<uint32_t>(NumIndices);

	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		for (const size_t Index : SubMesh->Indices) {
			auto Idx = static_cast<uint16_t>(Index);
			ChunkWriter->Write(&Idx, sizeof(uint16_t));
		}
	}
	
	*ChunkWriter << static_cast<uint32_t>(Mesh->SubMeshes.size());

	for (const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
		*ChunkWriter << static_cast<uint32_t>(SubMesh->Indices.size() / 3);
	}
	
	return true;
}

bool ExporterExportKFNode(IGameNode* Node, const MPKFExporterOptions& ExportOptions, MPMemoryChunkWriter* ChunkWriter)
{
	WRITE_ASCII_STRING(NodeName, Node->GetName(), ChunkWriter)

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

	float Scale = 1.f;

	if (ExportOptions.Scale) {
		Scale = ExportOptions.ScaleValue;
	}

	const MPMatrix4x3 NodeMatrix(
		LocalMatrix[0].x, LocalMatrix[0].y, LocalMatrix[0].z,
		LocalMatrix[1].x, LocalMatrix[1].y, LocalMatrix[1].z,
		LocalMatrix[2].x, LocalMatrix[2].y, LocalMatrix[2].z,
		LocalMatrix[3].x * Scale, LocalMatrix[3].y * Scale, LocalMatrix[3].z * Scale);

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

bool ExporterExportKFMesh(const MPKFMesh* Mesh,  const MPKFExporterOptions& ExportOptions, MPMemoryWriter& MemoryWriter)
{
	assert(Mesh);

	std::unique_ptr<MPMemoryChunkWriter> ChunkWriter{ MemoryWriter.CreateChunk(0x0C, MPKFTYPE_MESH_CHUNK_ID, 2) };
	
	//Node
	std::unique_ptr<MPMemoryChunkWriter> NodeChunk{ MemoryWriter.CreateChunk(0x0C, MPKFTYPE_NODE_SUB_CHUNK_ID, 1) };
	if (!ExporterExportKFNode(Mesh->Node, ExportOptions, NodeChunk.get())) {
		return false;
	}
	*ChunkWriter << NodeChunk.get();
	NodeChunk.reset();

	//Geometry
	std::unique_ptr<MPMemoryChunkWriter> GeometryChunk{ MemoryWriter.CreateChunk(0x0C, MPKFTYPE_GEOMETRY_SUB_CHUNK_ID, 1) };
	if (!ExporterExportKFGeometry(Mesh, ExportOptions, GeometryChunk.get())) {
		return false;
	}
	*ChunkWriter << GeometryChunk.get();
	GeometryChunk.reset();

	//Polygons
	std::unique_ptr<MPMemoryChunkWriter> PolygonsChunk{ MemoryWriter.CreateChunk(0x0C, MPKFTYPE_POLYGONS_SUB_CHUNK_ID, 1) };
	if (!ExporterExportKFPolygons(Mesh, ExportOptions, PolygonsChunk.get())) {
		return false;
	}
	*ChunkWriter << PolygonsChunk.get();
	PolygonsChunk.reset();

	//Polygon Materials
	std::unique_ptr<MPMemoryChunkWriter> PolygonMaterialsChunk{ MemoryWriter.CreateChunk(0x0C, MPKFTYPE_POLYGON_MATERIAL_SUB_CHUNK_ID, 1) };
	if (!ExporterExportKFPolygonMaterials(Mesh, ExportOptions, PolygonMaterialsChunk.get())) {
		return false;
	}
	*ChunkWriter << PolygonMaterialsChunk.get();
	PolygonMaterialsChunk.reset();

	//Polygon UVW
	if (ExportOptions.ExportMaterials) {
		std::unique_ptr<MPMemoryChunkWriter> UVChunk{ MemoryWriter.CreateChunk(0x0C, MPKFTYPE_UV_MAPPING_SUB_CHUNK_ID, 1) };
		if (!ExporterExportKFUVWMapping(Mesh, ExportOptions, UVChunk.get())) {
			return false;
		}
		*ChunkWriter << UVChunk.get();
		UVChunk.reset();
	}

	MemoryWriter << ChunkWriter.get();
	ChunkWriter.reset();
	
	return true;
}

bool ExporterExportKFMeshes(const MPKFScene* Scene, const MPKFExporterOptions& ExportOptions, MPMemoryWriter& MemoryWriter)
{
	assert(Scene);

	for (const std::unique_ptr<MPKFSceneNode>& SceneNode : Scene->Nodes)
	{
		if (SceneNode->Type != MPKFSCENE_TYPE::kMesh) {
			continue;
		}

		if (const auto Mesh = dynamic_cast<MPKFMesh*>(SceneNode.get()); !ExporterExportKFMesh(Mesh, ExportOptions, MemoryWriter)) {
			return false;
		}
	}
	
	return true;
}

// Export Materials
bool ExportKFTexture(Texmap* Texture, const TSTR& CopyDirTo, const MPKFExporterOptions& ExportOptions, MPMemoryChunkWriter* ChunkWriter)
{
	assert(Texture);

	if (Texture->ClassID() != MPKFTEXTURE_CLASS_ID) {
		TSTR Str;
		Texture->GetClassName(Str);
		SHOW_ERROR(MPKFEXPORTER_ERROR_UNSUPORTED_TEXTURE, Texture->GetName().data(), Str.data())
		return false;
	}

	const std::unique_ptr<MPMemoryChunkWriter> TextureChunkWriter{ ChunkWriter->CreateChunk(0x0C, MPKFTYPE_TEXTURE_SUB_CHUNK_ID, 1)};
	const auto KFTexture = dynamic_cast<IMPKFTexture*>(Texture);

	const int32_t NumberOfTextures = KFTexture->GetTexturesCount();

	if (NumberOfTextures <= 0) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_TEXTURE_EMPTY_BITMAP, KFTexture->GetTextureName())
		return false;
	}

	if (NumberOfTextures > 1 && KFTexture->GetAnimationStartFrame() > NumberOfTextures) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_TEXTURE_START_FRAME, KFTexture->GetTextureName(), NumberOfTextures, KFTexture->GetAnimationStartFrame());
		return false;
	}

	WRITE_ASCII_STRING(TextureName, KFTexture->GetTextureName(), TextureChunkWriter)

	if (KFTexture->IsMipMapsAuto()) {
		*TextureChunkWriter << static_cast<int32_t>(0);
	} else {
		*TextureChunkWriter << KFTexture->GetMipMapsNum();
	}

	*TextureChunkWriter << static_cast<int32_t>(KFTexture->GetFiltering());
	*TextureChunkWriter << NumberOfTextures;

	for (int32_t TextureIndex = 0; TextureIndex != NumberOfTextures; TextureIndex++) {
		const TSTR TextureFileName = KFTexture->GetTextureFileName(TextureIndex);

		if (TextureFileName == _T("")) {
			SHOW_ERROR(MPKFEXPORTER_ERROR_TEXTURE_EMPTY_BITMAP, KFTexture->GetTextureName());
			return false;
		}

		IS_ASCII_STRING(TextureFileName.data())

		TSTR FilePath, FileName, FileExt;
		SplitFilename(TextureFileName, &FilePath, &FileName, &FileExt);

		TSTR FileNameWithExt(FileName + FileExt);

		if (ExportOptions.CopyTexturesToExportPath) {
			TSTR CopyPath(CopyDirTo);
			CopyPath.Append(_T("\\")).Append(FileNameWithExt);
			CopyFile(TextureFileName, CopyPath.data(), FALSE);
		}

		CStr FileNameAsc = FileNameWithExt.ToCStr();
		*TextureChunkWriter << MPString(FileNameAsc.data(), FileNameAsc.Length());
	}

	if (NumberOfTextures > 1) {
		*TextureChunkWriter << KFTexture->IsAnimationAutomaticStart();
		*TextureChunkWriter << KFTexture->IsAnimationRandomStartFrame();
		*TextureChunkWriter << KFTexture->GetAnimationStartFrame();
		*TextureChunkWriter << KFTexture->GetAnimationFPS();
		*TextureChunkWriter << static_cast<int32_t>(KFTexture->GetAnimationEndCondition());
	}

	*ChunkWriter << TextureChunkWriter.get();

	return true;
}

bool ExportKFMaterial(IGameMaterial* Material, const TSTR& CopyDirTo, const MPKFExporterOptions& ExportOptions, MPMemoryWriter& MemoryWriter)
{
	assert(Material);
	
	Mtl* MaxMaterial = Material->GetMaxMaterial();
	
	if (MaxMaterial->ClassID() != MPKFMATERIAL_CLASS_ID) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_UNSUPORTED_MATERIAL, Material->GetMaterialName(), Material->GetClassName());
		return false;
	}

	IS_ASCII_STRING(Material->GetMaterialName());

	auto* KFMaterial = dynamic_cast<IMPKFMaterial*>(MaxMaterial);

	if (static_cast<int>(KFMaterial->GetGameVersion()) != ExportOptions.Game) {
		SHOW_ERROR(MPKFEXPORTER_ERROR_MATERIAL_WRONG_TARGET, ExportOptions.Game == 0 ? _T("Max Payne 1") : _T("Max Payne 2"), Material->GetMaterialName(), KFMaterial->GetGameVersion() == KFMaterialGameVersion::kMaxPayne1 ? _T("Max Payne 1") : _T("Max Payne 2"));
		return false;
	}

	std::unique_ptr<MPMemoryChunkWriter> ChunkWriter{ MemoryWriter.CreateChunk(0x0C, MPKFTYPE_MATERIAL_SUB_CHUNK_ID, ExportOptions.Game == 0 ? 1 : 2) };
	
	WRITE_ASCII_STRING(MaterialName, Material->GetMaterialName(), ChunkWriter)

	*ChunkWriter << KFMaterial->IsTwoSided();
	*ChunkWriter << KFMaterial->IsFogging();
	*ChunkWriter << false; //is_diffuse_combined
	*ChunkWriter << KFMaterial->IsInvisibleGeometry();
	*ChunkWriter << KFMaterial->HasVertexAlpha();
	*ChunkWriter << static_cast<int32_t>(KFMaterial->GetDiffuseColorShadingType());
	*ChunkWriter << static_cast<int32_t>(KFMaterial->GetSpecularColorShadingType());
	*ChunkWriter << static_cast<int32_t>(KFMaterial->GetLitShadingType());

	const MPColor AmbientColor = KFMaterial->GetAmbientColor();
	*ChunkWriter << AmbientColor.R;
	*ChunkWriter << AmbientColor.G;
	*ChunkWriter << AmbientColor.B;
	*ChunkWriter << AmbientColor.A;

	const MPColor DiffuseColor = KFMaterial->GetDiffuseColor();
	*ChunkWriter << DiffuseColor.R;
	*ChunkWriter << DiffuseColor.G;
	*ChunkWriter << DiffuseColor.B;
	*ChunkWriter << DiffuseColor.A;

	const MPColor SpecularColor = KFMaterial->GetSpecularColor();
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
		if (!ExportKFTexture(KFMaterial->GetDiffuseTexture(), CopyDirTo, ExportOptions, ChunkWriter.get())) {
			return false;
		}
	}

	*ChunkWriter << KFMaterial->HasReflectionTexture();
	if (KFMaterial->HasReflectionTexture()) {
		if (!KFMaterial->GetReflectionTexture()) {
			SHOW_ERROR(MPKFEXPORTER_ERROR_TEXTURE_EMPTY, _T("Reflection"), MaterialName.data());
			return false;
		}
		if (!ExportKFTexture(KFMaterial->GetReflectionTexture(), CopyDirTo, ExportOptions, ChunkWriter.get())) {
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
			if (!ExportKFTexture(KFMaterial->GetBumpTexture(), CopyDirTo, ExportOptions, ChunkWriter.get())) {
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
			if (!ExportKFTexture(KFMaterial->GetAlphaTexture(), CopyDirTo, ExportOptions, ChunkWriter.get())) {
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
			if (!ExportKFTexture(KFMaterial->GetMaskTexture(), CopyDirTo, ExportOptions, ChunkWriter.get())) {
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
		*ChunkWriter << static_cast<int32_t>(KFMaterial->GetAlphaReferenceValue());
	}

	MemoryWriter << ChunkWriter.get();
	ChunkWriter.reset();
	
	return true;
}

bool ExporterExportKFMaterials(const MPKFScene* Scene, const TSTR& CopyDirTo, const MPKFExporterOptions& ExportOptions, MPMemoryWriter& MemoryWriter)
{
	assert(Scene);
	
	MPMemoryWriter MaterialsWriter;
	std::vector<IGameMaterial*> GameMaterials;
	for (const std::unique_ptr<MPKFSceneNode>& SceneNode : Scene->Nodes)
	{
		if (SceneNode->Type != MPKFSCENE_TYPE::kMesh) {
			continue;
		}

		for (const auto Mesh = dynamic_cast<MPKFMesh*>(SceneNode.get()); const std::unique_ptr<MPKFSubMesh>& SubMesh : Mesh->SubMeshes) {
			if (auto Result = std::ranges::find(GameMaterials, SubMesh->Material); Result != std::end(GameMaterials)) {
				continue;
			}
			GameMaterials.push_back(SubMesh->Material);
			if (!ExportKFMaterial(SubMesh->Material, CopyDirTo, ExportOptions, MaterialsWriter)) {
				return false;
			}
		}
	}

	std::unique_ptr<MPMemoryChunkWriter> MaterialListChunk{ MemoryWriter.CreateChunk(0x0C, MPKFTYPE_MATERIAL_LIST_CHUNK_ID, 0) };

	const CStr PathsAsc = ExportOptions.Paths.ToCStr();
	*MaterialListChunk << MPString(PathsAsc.data(), PathsAsc.Length());
	*MaterialListChunk << static_cast<int32_t>(GameMaterials.size());
	MaterialListChunk->Write(MaterialsWriter.GetData(), MaterialsWriter.GetSize());
	MemoryWriter << MaterialListChunk.get();
	MaterialListChunk.reset();
	
	return true;
}
