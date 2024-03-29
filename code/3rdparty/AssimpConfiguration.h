#pragma once

//#define ASSIMP_BUILD_NO_X_IMPORTER
#define ASSIMP_BUILD_NO_C4D_IMPORTER
#define ASSIMP_BUILD_NO_AMF_IMPORTER
#define ASSIMP_BUILD_NO_3DS_IMPORTER
#define ASSIMP_BUILD_NO_MD3_IMPORTER
#define ASSIMP_BUILD_NO_MDL_IMPORTER
#define ASSIMP_BUILD_NO_MD2_IMPORTER
#define ASSIMP_BUILD_NO_PLY_IMPORTER
#define ASSIMP_BUILD_NO_ASE_IMPORTER
// #define ASSIMP_BUILD_NO_OBJ_IMPORTER
#define ASSIMP_BUILD_NO_HMP_IMPORTER
#define ASSIMP_BUILD_NO_SMD_IMPORTER
#define ASSIMP_BUILD_NO_MDC_IMPORTER
// #define ASSIMP_BUILD_NO_MD5_IMPORTER
#define ASSIMP_BUILD_NO_STL_IMPORTER
#define ASSIMP_BUILD_NO_LWO_IMPORTER
#define ASSIMP_BUILD_NO_DXF_IMPORTER
#define ASSIMP_BUILD_NO_NFF_IMPORTER
#define ASSIMP_BUILD_NO_RAW_IMPORTER
#define ASSIMP_BUILD_NO_SIB_IMPORTER
#define ASSIMP_BUILD_NO_OFF_IMPORTER
#define ASSIMP_BUILD_NO_AC_IMPORTER
#define ASSIMP_BUILD_NO_BVH_IMPORTER
#define ASSIMP_BUILD_NO_IRRMESH_IMPORTER
#define ASSIMP_BUILD_NO_IRR_IMPORTER
#define ASSIMP_BUILD_NO_Q3D_IMPORTER
#define ASSIMP_BUILD_NO_B3D_IMPORTER
// #define ASSIMP_BUILD_NO_COLLADA_IMPORTER
#define ASSIMP_BUILD_NO_TERRAGEN_IMPORTER
#define ASSIMP_BUILD_NO_CSM_IMPORTER
#define ASSIMP_BUILD_NO_3D_IMPORTER
#define ASSIMP_BUILD_NO_LWS_IMPORTER
#define ASSIMP_BUILD_NO_OGRE_IMPORTER
#define ASSIMP_BUILD_NO_OPENGEX_IMPORTER
#define ASSIMP_BUILD_NO_MS3D_IMPORTER
#define ASSIMP_BUILD_NO_COB_IMPORTER
// #define ASSIMP_BUILD_NO_BLEND_IMPORTER
#define ASSIMP_BUILD_NO_Q3BSP_IMPORTER
#define ASSIMP_BUILD_NO_NDO_IMPORTER
#define ASSIMP_BUILD_NO_IFC_IMPORTER
#define ASSIMP_BUILD_NO_XGL_IMPORTER
// #define ASSIMP_BUILD_NO_FBX_IMPORTER
#define ASSIMP_BUILD_NO_ASSBIN_IMPORTER
// #define ASSIMP_BUILD_NO_GLTF_IMPORTER
#define ASSIMP_BUILD_NO_GLTF1_IMPORTER
#define ASSIMP_BUILD_NO_3MF_IMPORTER
#define ASSIMP_BUILD_NO_X3D_IMPORTER
#define ASSIMP_BUILD_NO_MMD_IMPORTER
#define ASSIMP_BUILD_NO_M3D_IMPORTER
#define ASSIMP_BUILD_NO_STEP_IMPORTER

// Exporter
#define ASSIMP_BUILD_NO_COLLADA_EXPORTER
#define ASSIMP_BUILD_NO_X_EXPORTER
#define ASSIMP_BUILD_NO_STEP_EXPORTER
#define ASSIMP_BUILD_NO_OBJ_EXPORTER
#define ASSIMP_BUILD_NO_STL_EXPORTER
#define ASSIMP_BUILD_NO_PLY_EXPORTER
#define ASSIMP_BUILD_NO_3DS_EXPORTER
#define ASSIMP_BUILD_NO_GLTF_EXPORTER
#define ASSIMP_BUILD_NO_ASSBIN_EXPORTER
#define ASSIMP_BUILD_NO_ASSXML_EXPORTER
#define ASSIMP_BUILD_NO_X3D_EXPORTER
#define ASSIMP_BUILD_NO_FBX_EXPORTER
#define ASSIMP_BUILD_NO_3MF_EXPORTER
#define ASSIMP_BUILD_NO_M3D_EXPORTER
#define ASSIMP_BUILD_NO_ASSJSON_EXPORTER

// Amalgamated/unity build
#ifdef _WIN32
	// Disable warnings in external headers, we can't fix them
__pragma(warning(disable: 4355))	// warning C4355: 'this': used in base member initializer list
__pragma(warning(disable: 4619))	// warning C4619: #pragma warning: there is no warning number '4351'
__pragma(warning(disable: 4777))	// warning C4777: 'snprintf' : format string '%Iu' requires an argument of type 'unsigned int', but variadic argument 1 has type 'const size_t'
__pragma(warning(disable: 4826))	// warning C4826: Conversion from 'aiBone **' to 'uint64_t' is sign-extended. This may cause unexpected runtime behavior.
#endif
