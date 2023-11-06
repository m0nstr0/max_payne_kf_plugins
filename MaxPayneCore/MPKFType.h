#pragma once
#include <cstdint>
#include "MPType.h"
#include <vector>

struct MPKFMesh {
    std::vector<MPVector3> Vertices;
    std::vector<MPVector3> Normals;
    std::vector<uint32_t> VerticesPerPrimitive;
};

/*
struct MPKFTypeTexture
{
    M_String texture_name;
    int32_t NumberOfMipMaps;
    int32_t Filtering;
    int32_t NumberOfTextures;
    M_String texture_file_name[number_of_textures];
    bool AutomaticStart;
    bool RandomStartFrame;
    int32_t StartFrame;
    int32_t PlaybackFPS;
    int32_t EndCondition;
};

struct MPKFTypeMaterial
{
    M_String material_name;
    bool is_two_sided;
    bool is_fogging;
    bool is_diffuse_combined;
    bool is_invisible_geometry;
    bool has_vertex_alpha;
    int32_t diffuse_color_type; //0x00 None 0x01 Color 0x02 Gourand
    int32_t specular_color_type; //0x00 None 0x01 Gourand
    int32_t lit_type; //0x00 Phong 0x01 Environment 0x02 UVW2
    float ambient_color_r;
    float ambient_color_g;
    float ambient_color_b;
    float ambient_color_a;
    float diffuse_color_r;
    float diffuse_color_g;
    float diffuse_color_b;
    float diffuse_color_a;
    float specular_color_r;
    float specular_color_g;
    float specular_color_b;
    float specular_color_a;
    float vertex_alpha;
    float specular_exponent;
    int32_t diffuse_texture_type; //0x00 Copy 0x01 Additive 0x02 Multiplicative
    int32_t reflection_texture_type; //0x00 Copy 0x01 Additive 0x02 Multiplicative
    float emboss_factor;
    bool has_diffuse_texture;
    if (has_diffuse_texture) {
        SubChunk_Texture diffuse_texture_sub_chunk;
    }
    bool has_reflection_texture;
        if (has_reflection_texture) {
            SubChunk_Texture reflection_texture_sub_chunk;
        }
    bool has_bump_texture;
        if (has_bump_texture) {
            SubChunk_Texture bump_texture_sub_chunk;
        }
    bool has_opacity_texture;
        if (has_opacity_texture) {
            SubChunk_Texture opacity_texture_sub_chunk;
        }
};

struct MPKFTypeMaterialList
{
    M_String texture_dirs;
    M_Int number_of_materials;
    SubChunk_Material material_sub_chunk[number_of_materials];
};*/