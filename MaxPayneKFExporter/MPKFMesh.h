#pragma once

#include <map>
#include <MPType.h>
#include <vector>
#include "MaxPayneKFExporter.h"
#include "MPKFScene.h"
#include "MPMemoryWriter.h"
#include "MPKFSkin.h"

#define	MPKFEXPORTER_ERROR_MESH_TRIANGULATE					_T("Unable to triangulate mesh \"{}\"")
#define MPKFEXPORTER_ERROR_UNSUPORTED_MATERIAL				_T("Material \"{}\" has unsupported class \"{}\", only KF Materials and Multi/Sub-Object are allowed")
#define MPKFEXPORTER_ERROR_UNSUPORTED_TEXTURE				_T("Texture \"{}\" has unsupported class \"{}\", only KF Textures are allowed")
#define	MPKFEXPORTER_ERROR_MATERIAL_WRONG_TARGET			_T("Export target is \"{}\" but the material's \"{}\" target is \"{}\"")
#define	MPKFEXPORTER_ERROR_TEXTURE_EMPTY					_T("There is no texture in the slot \"{}\" in the material \"{}\"")
#define	MPKFEXPORTER_ERROR_TEXTURE_EMPTY_BITMAP				_T("There is no bitmap in the texture \"{}\"")
#define MPKFEXPORTER_ERROR_TEXTURE_START_FRAME				_T("Texture \"{}\" has only \"{}\" frames but the field Start Frame is set to \"{}\"")
#define MPKFEXPORTER_ERROR_OBJECTS_WITHOUT_MATERIAL			_T("Object \"{}\" doesn't have any material")

struct MPKFSubMesh
{
    IGameMaterial* Material;

    std::vector<MPVector3> Vertices;

    std::vector<MPVector3> Normals;

    std::vector<MPVector3> UVs;

    std::vector<uint32_t> Indices;

    std::map<size_t, int> LocalToOrigVertexId;

    explicit MPKFSubMesh(IGameMaterial* InMaterial): Material{InMaterial} {}
};

class MPKFSkin;

struct MPKFMesh: public MPKFSceneNode
{
    size_t OriginalMeshNumVertices;

    std::vector<std::unique_ptr<MPKFSubMesh>> SubMeshes;

    std::unique_ptr<MPKFSkin> Skin;
    
    explicit MPKFMesh(IGameNode* InNode) :
        OriginalMeshNumVertices{ 0 }, 
        MPKFSceneNode(InNode, MPKFSCENE_TYPE::kMesh), 
        Skin{ nullptr }
    {}

    ~MPKFMesh() override = default;
    
    MPKFSubMesh* FindOrCreateSubMeshByMaterial(IGameMaterial* Material)
    {
        const auto Result = std::ranges::find_if(SubMeshes, [Material](const std::unique_ptr<MPKFSubMesh>& SubMesh) {
           return SubMesh->Material == Material;
        });

        if (Result != std::end(SubMeshes)) {
            return Result->get();
        }

        SubMeshes.push_back(std::make_unique<MPKFSubMesh>(Material));
        
        return SubMeshes.back().get();
    }
};

bool ExporterCreateKFMesh(IGameNode* Node, IGameObject* GameObject, IGameMesh* Mesh, MPKFMesh** OutMesh);

bool ExporterExportKFMeshes(const MPKFScene* Scene, const MPKFExporterOptions& ExportOptions, MPMemoryWriter& MemoryWriter);

bool ExporterExportKFMaterials(const MPKFScene* Scene, const TSTR& CopyDirTo, const MPKFExporterOptions& ExporterOptions, MPMemoryWriter& MemoryWriter);