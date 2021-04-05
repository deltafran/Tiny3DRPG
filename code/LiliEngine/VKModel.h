#pragma once

#include "VKCommandBuffer.h"
#include "VKBuffer.h"
#include "VKIndexBuffer.h"
#include "VKVertexBuffer.h"
#include "RHIDefinitions.h"

#include "CoreMath2.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Quat.h"

struct aiMesh;
struct aiScene;
struct aiNode;

struct VKNode;

struct VKBoundingBox
{
	Vector3 min;
	Vector3 max;
	Vector3 corners[8];

	VKBoundingBox()
		: min(MAX_flt, MAX_flt, MAX_flt)
		, max(MIN_flt, MIN_flt, MIN_flt)
	{

	}

	VKBoundingBox(const Vector3& inMin, const Vector3& inMax)
		: min(inMin)
		, max(inMax)
	{

	}

	void UpdateCorners()
	{
		corners[0].Set(min.x, min.y, min.z);
		corners[1].Set(max.x, min.y, min.z);
		corners[2].Set(min.x, max.y, min.z);
		corners[3].Set(max.x, max.y, min.z);

		corners[4].Set(min.x, min.y, max.z);
		corners[5].Set(max.x, min.y, max.z);
		corners[6].Set(min.x, max.y, max.z);
		corners[7].Set(max.x, max.y, max.z);
	}
};

struct VKPrimitive
{
	VKIndexBuffer* indexBuffer = nullptr;
	VKVertexBuffer* vertexBuffer = nullptr;
	VKVertexBuffer* instanceBuffer = nullptr;

	std::vector<float>	vertices;
	std::vector<float>  instanceDatas;
	std::vector<uint16_t>	indices;

	int32_t               vertexCount = 0;
	int32_t               triangleNum = 0;

	VKPrimitive()
	{

	}

	~VKPrimitive()
	{
		if (indexBuffer) {
			delete indexBuffer;
		}

		if (vertexBuffer) {
			delete vertexBuffer;
		}

		if (instanceBuffer) {
			delete instanceBuffer;
		}

		indexBuffer = nullptr;
		vertexBuffer = nullptr;
	}

	void DrawOnly(VkCommandBuffer cmdBuffer)
	{
		if (vertexBuffer && !indexBuffer) {
			vkCmdDraw(cmdBuffer, vertexCount, 1, 0, 0);
		}
		else {
			vkCmdDrawIndexed(cmdBuffer, indexBuffer->indexCount, indexBuffer->instanceCount, 0, 0, 0);
		}
	}

	void BindOnly(VkCommandBuffer cmdBuffer)
	{
		if (vertexBuffer) {
			vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &(vertexBuffer->dvkBuffer->buffer), &(vertexBuffer->offset));
		}

		if (instanceBuffer) {
			vkCmdBindVertexBuffers(cmdBuffer, 1, 1, &(instanceBuffer->dvkBuffer->buffer), &(instanceBuffer->offset));
		}

		if (indexBuffer) {
			vkCmdBindIndexBuffer(cmdBuffer, indexBuffer->dvkBuffer->buffer, 0, indexBuffer->indexType);
		}
	}

	void BindDrawCmd(VkCommandBuffer cmdBuffer)
	{
		if (vertexBuffer) {
			vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &(vertexBuffer->dvkBuffer->buffer), &(vertexBuffer->offset));
		}

		if (instanceBuffer) {
			vkCmdBindVertexBuffers(cmdBuffer, 1, 1, &(instanceBuffer->dvkBuffer->buffer), &(instanceBuffer->offset));
		}

		if (indexBuffer) {
			vkCmdBindIndexBuffer(cmdBuffer, indexBuffer->dvkBuffer->buffer, 0, indexBuffer->indexType);
		}

		if (vertexBuffer && !indexBuffer) {
			vkCmdDraw(cmdBuffer, vertexCount, 1, 0, 0);
		}
		else {
			vkCmdDrawIndexed(cmdBuffer, indexBuffer->indexCount, indexBuffer->instanceCount, 0, 0, 0);
		}
	}
};

struct VKMaterialInfo
{
	std::string		diffuse;
	std::string		normalmap;
	std::string		specular;
};

struct VKBone
{
	std::string     name;
	int32_t           index = -1;
	int32_t           parent = -1;
	Matrix4x4       inverseBindPose;
	Matrix4x4		finalTransform;
};

struct VKVertexSkin
{
	int32_t  used = 0;
	int32_t  indices[4];
	float  weights[4];
};

template<class ValueType>
struct VKAnimChannel
{
	std::vector<float>	   keys;
	std::vector<ValueType> values;

	void GetValue(float key, ValueType& outPrevValue, ValueType& outNextValue, float& outAlpha)
	{
		outAlpha = 0.0f;

		if (keys.size() == 0) {
			return;
		}

		if (key <= keys.front())
		{
			outPrevValue = values.front();
			outNextValue = values.front();
			outAlpha = 0.0f;
			return;
		}

		if (key >= keys.back())
		{
			outPrevValue = values.back();
			outNextValue = values.back();
			outAlpha = 0.0f;
			return;
		}

		int32_t frameIndex = 0;
		for (int32_t i = 0; i < keys.size() - 1; ++i) {
			if (key <= keys[i + 1])
			{
				frameIndex = i;
				break;
			}
		}

		outPrevValue = values[frameIndex + 0];
		outNextValue = values[frameIndex + 1];

		float prevKey = keys[frameIndex + 0];
		float nextKey = keys[frameIndex + 1];
		outAlpha = (key - prevKey) / (nextKey - prevKey);
	}
};

struct VKAnimationClip
{
	std::string					nodeName;
	float						duration;
	VKAnimChannel<Vector3>		positions;
	VKAnimChannel<Vector3>		scales;
	VKAnimChannel<Quat>		rotations;
};

struct VKAnimation
{
	std::string name;
	float		time = 0.0f;
	float       duration = 0.0f;
	float		speed = 1.0f;
	std::unordered_map<std::string, VKAnimationClip> clips;
};

struct VKMesh
{
	typedef std::vector<VKPrimitive*> VKPrimitives;

	VKPrimitives		primitives;
	VKBoundingBox		bounding;
	VKNode* linkNode;

	std::vector<int32_t>	bones;
	bool				isSkin = false;

	VKMaterialInfo		material;

	int32_t				vertexCount;
	int32_t				triangleCount;

	VKMesh()
		: linkNode(nullptr)
		, vertexCount(0)
		, triangleCount(0)
	{

	}

	void BindOnly(VkCommandBuffer cmdBuffer)
	{
		for (int i = 0; i < primitives.size(); ++i) {
			primitives[i]->BindOnly(cmdBuffer);
		}
	}

	void DrawOnly(VkCommandBuffer cmdBuffer)
	{
		for (int i = 0; i < primitives.size(); ++i) {
			primitives[i]->DrawOnly(cmdBuffer);
		}
	}

	void BindDrawCmd(VkCommandBuffer cmdBuffer)
	{
		for (int i = 0; i < primitives.size(); ++i) {
			primitives[i]->BindDrawCmd(cmdBuffer);
		}
	}

	~VKMesh()
	{
		for (int i = 0; i < primitives.size(); ++i) {
			delete primitives[i];
		}
		primitives.clear();
		linkNode = nullptr;
	}
};

struct VKNode
{
	std::string					name;

	std::vector<VKMesh*>		meshes;

	VKNode* parent;
	std::vector<VKNode*>		children;

	Matrix4x4					localMatrix;
	Matrix4x4					globalMatrix;

	VKNode()
		: name("None")
		, parent(nullptr)
	{

	}

	const Matrix4x4& GetLocalMatrix()
	{
		return localMatrix;
	}

	Matrix4x4& GetGlobalMatrix()
	{
		globalMatrix = localMatrix;

		if (parent) {
			globalMatrix.Append(parent->GetGlobalMatrix());
		}

		return globalMatrix;
	}

	void CalcBounds(VKBoundingBox& outBounds)
	{
		if (meshes.size() > 0)
		{
			const Matrix4x4& matrix = GetGlobalMatrix();
			for (int32_t i = 0; i < meshes.size(); ++i)
			{
				Vector3 mmin = matrix.TransformPosition(meshes[i]->bounding.min);
				Vector3 mmax = matrix.TransformPosition(meshes[i]->bounding.max);
				outBounds.min = Vector3::Min(outBounds.min, mmin);
				outBounds.min = Vector3::Min(outBounds.min, mmax);
				outBounds.max = Vector3::Max(outBounds.max, mmin);
				outBounds.max = Vector3::Max(outBounds.max, mmax);
			}
		}

		for (int32_t i = 0; i < children.size(); ++i) {
			children[i]->CalcBounds(outBounds);
		}
	}

	VKBoundingBox GetBounds()
	{
		VKBoundingBox bounds;
		bounds.min.Set(MAX_int32, MAX_int32, MAX_int32);
		bounds.max.Set(-MAX_int32, -MAX_int32, -MAX_int32);
		CalcBounds(bounds);
		bounds.UpdateCorners();
		return bounds;
	}

	~VKNode()
	{
		for (int32_t i = 0; i < meshes.size(); ++i) {
			delete meshes[i];
		}
		meshes.clear();

		for (int32_t i = 0; i < children.size(); ++i) {
			delete children[i];
		}
		children.clear();
	}
};

class VKModel
{
private:
	VKModel()
		: device(nullptr)
		, rootNode(nullptr)
	{

	}

public:
	~VKModel()
	{
		delete rootNode;
		rootNode = nullptr;
		device = nullptr;

		meshes.clear();
		linearNodes.clear();

		for (int32_t i = 0; i < bones.size(); ++i) {
			delete bones[i];
		}
		bones.clear();
	}

	void Update(float time, float delta);

	void SetAnimation(int32_t index);

	VKAnimation& GetAnimation(int32_t index = -1);

	void GotoAnimation(float time);

	VkVertexInputBindingDescription GetInputBinding();

	std::vector<VkVertexInputAttributeDescription> GetInputAttributes();

	static VKModel* LoadFromFile(const std::string& filename, std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, const std::vector<VertexAttribute>& attributes);

	static VKModel* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VKCommandBuffer* cmdBuffer, const std::vector<float>& vertices, const std::vector<uint16_t>& indices, const std::vector<VertexAttribute>& attributes);

protected:

	VKNode* LoadNode(const aiNode* node, const aiScene* scene);

	VKMesh* LoadMesh(const aiMesh* mesh, const aiScene* scene);

	void LoadBones(const aiScene* aiScene);

	void LoadSkin(std::unordered_map<uint32_t, VKVertexSkin>& skinInfoMap, VKMesh* mesh, const aiMesh* aiMesh, const aiScene* aiScene);

	void LoadVertexDatas(std::unordered_map<uint32_t, VKVertexSkin>& skinInfoMap, std::vector<float>& vertices, Vector3& mmax, Vector3& mmin, VKMesh* mesh, const aiMesh* aiMesh, const aiScene* aiScene);

	void LoadIndices(std::vector<uint32_t>& indices, const aiMesh* aiMesh, const aiScene* aiScene);

	void LoadPrimitives(std::vector<float>& vertices, std::vector<uint32_t>& indices, VKMesh* mesh, const aiMesh* aiMesh, const aiScene* aiScene);

	void LoadAnim(const aiScene* aiScene);

public:
	typedef std::unordered_map<std::string, VKNode*> NodesMap;
	typedef std::unordered_map<std::string, VKBone*> BonesMap;

	std::shared_ptr<VulkanDevice>	device;

	VKNode* rootNode;
	std::vector<VKNode*>			linearNodes;
	std::vector<VKMesh*>			meshes;

	NodesMap						nodesMap;

	std::vector<VKBone*>			bones;
	BonesMap						bonesMap;

	std::vector<VertexAttribute>	attributes;
	std::vector<VKAnimation>		animations;
	int32_t							animIndex = -1;

private:

	VKCommandBuffer* cmdBuffer = nullptr;
	bool                            loadSkin = false;
};