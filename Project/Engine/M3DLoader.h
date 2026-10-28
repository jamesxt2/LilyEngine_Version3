#pragma once

#include "AnimData.h"

class M3DLoader
{
public:
    struct Vertex
    {
        Vector3 Pos;
        Vector3 Normal;
        Vector2 TexC;
        Vector4 TangentU;
    };

    struct Subset
    {
        UINT Id = -1;
        UINT VertexStart = 0;
        UINT VertexCount = 0;
        UINT FaceStart = 0;
        UINT FaceCount = 0;
    };

    struct M3dMaterial
    {
        std::wstring Name;

        Vector4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f };
        Vector3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
        float Roughness = 0.8f;
        bool AlphaClip = false;

        std::wstring MaterialTypeName;
        std::wstring DiffuseMapName;
        std::wstring NormalMapName;
    };

    bool LoadM3d(const std::wstring& filename,
        std::vector<M3DLoader::Vertex>& vertices,
        std::vector<uint16>& indices,
        std::vector<Subset>& subsets,
        std::vector<M3dMaterial>& mats);
    bool LoadM3d(const std::wstring& filename,
        std::vector<SkinnedVertex>& vertices,
        std::vector<uint16>& indices,
        std::vector<Subset>& subsets,
        std::vector<M3dMaterial>& mats,
        SkinnedData& skinInfo);

private:
    void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats);
    void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<Subset>& subsets);
    void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<M3DLoader::Vertex>& vertices);
    void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<SkinnedVertex>& vertices);
    void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices);
    void ReadBoneOffsets(std::ifstream& fin, UINT numBones, std::vector<Matrix>& boneOffsets);
    void ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex);
    void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::unordered_map<std::wstring, AnimationClip>& animations);
    void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation);
};

