#include "AssimpConfiguration.h"
// AssetLib
	// FBX
	#include "code/AssetLib/FBX/FBXImporter.cpp"
	#include "code/AssetLib/FBX/FBXParser.cpp"
	// PostProcessing
	#include "code/PostProcessing/ArmaturePopulate.cpp"
	#include "code/PostProcessing/CalcTangentsProcess.cpp"
	#undef small	// Get rid of some nasty macros
	#include "code/PostProcessing/ComputeUVMappingProcess.cpp"
	#include "code/PostProcessing/ConvertToLHProcess.cpp"
	#include "code/PostProcessing/DeboneProcess.cpp"
	#include "code/PostProcessing/DropFaceNormalsProcess.cpp"
	#include "code/PostProcessing/EmbedTexturesProcess.cpp"
	#include "code/PostProcessing/FindDegenerates.cpp"
	#include "code/PostProcessing/FindInstancesProcess.cpp"
	#include "code/PostProcessing/FindInvalidDataProcess.cpp"
	#include "code/PostProcessing/FixNormalsStep.cpp"
	#include "code/PostProcessing/GenBoundingBoxesProcess.cpp"
	#include "code/PostProcessing/GenFaceNormalsProcess.cpp"
	#include "code/PostProcessing/GenVertexNormalsProcess.cpp"
	#include "code/PostProcessing/ImproveCacheLocality.cpp"
	#include "code/PostProcessing/JoinVerticesProcess.cpp"
	#include "code/PostProcessing/LimitBoneWeightsProcess.cpp"
	#include "code/PostProcessing/MakeVerboseFormat.cpp"
	#include "code/PostProcessing/OptimizeGraph.cpp"
	#include "code/PostProcessing/OptimizeMeshes.cpp"
	#include "code/PostProcessing/PretransformVertices.cpp"
	#include "code/PostProcessing/ProcessHelper.cpp"
	#include "code/PostProcessing/RemoveRedundantMaterials.cpp"
	#include "code/PostProcessing/RemoveVCProcess.cpp"
	#include "code/PostProcessing/ScaleProcess.cpp"
	#include "code/PostProcessing/SortByPTypeProcess.cpp"
	#include "code/PostProcessing/SplitByBoneCountProcess.cpp"
	#include "code/PostProcessing/SplitLargeMeshes.cpp"
	#include "code/PostProcessing/TextureTransform.cpp"
	#include "code/PostProcessing/TriangulateProcess.cpp"
	#include "code/PostProcessing/ValidateDataStructure.cpp"
// contrib
	// irrXML
	#include "contrib/irrXML/irrXML.cpp"
	// poly2tri
	#include "contrib/poly2tri/poly2tri/common/shapes.cc"
	#include "contrib/poly2tri/poly2tri/sweep/advancing_front.cc"
	#include "contrib/poly2tri/poly2tri/sweep/cdt.cc"
	#include "contrib/poly2tri/poly2tri/sweep/sweep.cc"
	#include "contrib/poly2tri/poly2tri/sweep/sweep_context.cc"
