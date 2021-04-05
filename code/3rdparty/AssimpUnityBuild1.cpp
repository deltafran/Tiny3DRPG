#include "AssimpConfiguration.h"
// code
	// AssetLib
		// Blender
		#include "code/AssetLib/Blender/BlenderBMesh.cpp"
		#include "code/AssetLib/Blender/BlenderCustomData.cpp"
		#include "code/AssetLib/Blender/BlenderDNA.cpp"
		#include "code/AssetLib/Blender/BlenderLoader.cpp"
		#include "code/AssetLib/Blender/BlenderModifier.cpp"
		//#include "code/AssetLib/Blender/BlenderScene.cpp"		# Due to conflict moved into "AssimpUnityBuild2.cpp"
		#include "code/AssetLib/Blender/BlenderTessellator.cpp"
		// Collada
		#include "code/AssetLib/Collada/ColladaHelper.cpp"
		//#include "code/AssetLib/Collada/ColladaLoader.cpp"	# Due to conflict moved into "AssimpUnityBuild2.cpp"
		//#include "code/AssetLib/Collada/ColladaParser.cpp"	# Due to conflict moved into "AssimpUnityBuild3.cpp"
		// FBX
		#include "code/AssetLib/FBX/FBXAnimation.cpp"
		#include "code/AssetLib/FBX/FBXBinaryTokenizer.cpp"
		#include "code/AssetLib/FBX/FBXConverter.cpp"
		#include "code/AssetLib/FBX/FBXDeformer.cpp"
		#include "code/AssetLib/FBX/FBXDocument.cpp"
		#include "code/AssetLib/FBX/FBXDocumentUtil.cpp"
		//#include "code/AssetLib/FBX/FBXImporter.cpp"		# Due to conflict moved into "AssimpUnityBuild5.cpp"
		#include "code/AssetLib/FBX/FBXMaterial.cpp"
		#include "code/AssetLib/FBX/FBXMeshGeometry.cpp"
		#include "code/AssetLib/FBX/FBXModel.cpp"
		#include "code/AssetLib/FBX/FBXNodeAttribute.cpp"
		//#include "code/AssetLib/FBX/FBXParser.cpp"	# Due to conflict moved into "AssimpUnityBuild5.cpp"
		#include "code/AssetLib/FBX/FBXProperties.cpp"
		#include "code/AssetLib/FBX/FBXTokenizer.cpp"
		#include "code/AssetLib/FBX/FBXUtil.cpp"
		// glTF
		#include "../RapidJSON/configuration/rapidjson/rapidjson.h"
		#include "code/AssetLib/glTF/glTFCommon.cpp"
		// glTF2
		//#include "code/AssetLib/glTF2/glTF2Importer.cpp"	# Due to conflict moved into "AssimpUnityBuild4.cpp"
		// MD5
		//#include "code/AssetLib/MD5/MD5Loader.cpp"	# Due to conflict moved into "AssimpUnityBuild2.cpp"
		#include "code/AssetLib/MD5/MD5Parser.cpp"
		// Obj
		//#include "code/AssetLib/Obj/ObjFileImporter.cpp"	# Due to conflict moved into "AssimpUnityBuild3.cpp"
		#include "code/AssetLib/Obj/ObjFileMtlImporter.cpp"
		#include "code/AssetLib/Obj/ObjFileParser.cpp"
	// CApi
	#include "code/CApi/AssimpCExport.cpp"
	#include "code/CApi/CInterfaceIOWrapper.cpp"
	// Common
	#include "code/Common/AssertHandler.cpp"
	#include "code/Common/Assimp.cpp"
	#include "code/Common/BaseImporter.cpp"
	#include "code/Common/BaseProcess.cpp"
	#include "code/Common/Bitmap.cpp"
	#include "code/Common/CreateAnimMesh.cpp"
	#include "code/Common/DefaultIOStream.cpp"
	#include "code/Common/DefaultIOSystem.cpp"
	#include "code/Common/DefaultLogger.cpp"
	#undef max	// Get rid of some nasty OS macros
	#include "code/Common/Exporter.cpp"
	#undef min				// Get rid of some nasty OS macros
	#undef CreateDirectory	// Get rid of some nasty OS macros
	#undef DeleteFile		// Get rid of some nasty OS macros
	#include "code/Common/Importer.cpp"
	#include "code/Common/ImporterRegistry.cpp"
	#include "code/Common/material.cpp"
	#include "code/Common/PostStepRegistry.cpp"
	#include "code/Common/RemoveComments.cpp"
	#include "code/Common/scene.cpp"
	#include "code/Common/SceneCombiner.cpp"
	#include "code/Common/ScenePreprocessor.cpp"
	#include "code/Common/SGSpatialSort.cpp"
	#include "code/Common/simd.cpp"
	#include "code/Common/SkeletonMeshBuilder.cpp"
	#include "code/Common/SpatialSort.cpp"
	#include "code/Common/StandardShapes.cpp"
	#include "code/Common/Subdivision.cpp"
	#include "code/Common/TargetAnimation.cpp"
	#include "code/Common/Version.cpp"
	#include "code/Common/VertexTriangleAdjacency.cpp"
	#include "code/Common/ZipArchiveIOSystem.cpp"
	// Material
	#include "code/Material/MaterialSystem.cpp"
