#ifndef KRISVERS_KGFX_ENUMERATIONS_H
#define KRISVERS_KGFX_ENUMERATIONS_H

/* result codes */
typedef enum {
	KGFX_SUCCESS = 0,
	KGFX_GENERIC_ERROR = 1,
	KGFX_VERSION_NOT_SUPPORTED = 2,
	KGFX_VERSION_MAJOR_MISMATCH = 3,
	KGFX_NULL_OUTPUT_ARGUMENT = 4,
	KGFX_INVALID_CONTEXT = 5,
	KGFX_NOT_IMPLEMENTED = 6,
	KGFX_INVALID_ARGUMENT = 7,
	KGFX_SIZE_TOO_LARGE = 8,
} KGFXresult;

typedef enum {
	KGFX_BACKEND_UNKNOWN = 0,
	KGFX_BACKEND_GENERIC = KGFX_BACKEND_UNKNOWN,
	KGFX_BACKEND_VULKAN = 1,
	KGFX_BACKEND_D3D12 = 2,
	KGFX_BACKEND_METAL = 3,
	KGFX_BACKEND_OPENGL = 4,
	KGFX_BACKEND_DUMMY = 255,
	KGFX_BACKEND_COUNT,
	KGFX_BACKEND_MAX = KGFX_BACKEND_COUNT - 1,
	KGFX_BACKEND_MIN = KGFX_BACKEND_UNKNOWN,
} KGFXbackend;

/* pipeline related enumerations */
typedef enum {
	KGFX_SHADERTYPE_VERTEX = 0,
	KGFX_SHADERTYPE_FRAGMENT = 1,
	KGFX_SHADERTYPE_GEOMETRY = 2,
	KGFX_SHADERTYPE_COMPUTE = 3,
	KGFX_SHADERTYPE_COUNT,
	KGFX_SHADERTYPE_MAX = KGFX_SHADERTYPE_COMPUTE,
	KGFX_SHADERTYPE_MIN = KGFX_SHADERTYPE_VERTEX,
} KGFXshadertype;

typedef enum {
	KGFX_MEDIUM_SPIRV = 0,
	KGFX_MEDIUM_GLSL = 1,
	KGFX_MEDIUM_HLSL = 2,
	KGFX_MEDIUM_MSL = 3,
} KGFXshadermedium;

typedef enum {
	KGFX_DATATYPE_FLOAT = 1,
	KGFX_DATATYPE_FLOAT2,
	KGFX_DATATYPE_FLOAT3,
	KGFX_DATATYPE_FLOAT4,
	KGFX_DATATYPE_INT,
	KGFX_DATATYPE_INT2,
	KGFX_DATATYPE_INT3,
	KGFX_DATATYPE_INT4,
	KGFX_DATATYPE_UINT,
	KGFX_DATATYPE_UINT2,
	KGFX_DATATYPE_UINT3,
	KGFX_DATATYPE_UINT4,
	KGFX_DATATYPE_MAT2,
	KGFX_DATATYPE_MAT3,
	KGFX_DATATYPE_MAT4,
	KGFX_DATATYPE_TEXTURE_SAMPLER,
	KGFX_DATATYPE_COUNT,
	KGFX_DATATYPE_MAX = KGFX_DATATYPE_COUNT - 1,
	KGFX_DATATYPE_MIN = KGFX_DATATYPE_FLOAT,
} KGFXdatatype;

typedef enum {
	KGFX_VERTEX_INPUT_RATE_VERTEX = 0,
	KGFX_VERTEX_INPUT_RATE_INSTANCE = 1,
} KGFXinputrate;

typedef enum {
	KGFX_BINDPOINT_VERTEX = 1,
	KGFX_BINDPOINT_FRAGMENT = 2,
	KGFX_BINDPOINT_GEOMETRY = 4,
	KGFX_BINDPOINT_COMPUTE = 8,
	KGFX_BINDPOINT_COUNT,
	KGFX_BINDPOINT_MAX = KGFX_BINDPOINT_COMPUTE,
	KGFX_BINDPOINT_MIN = KGFX_BINDPOINT_VERTEX,
} KGFXbindpoint;

typedef enum {
	KGFX_DESCRIPTOR_USAGE_INVALID = 0,
	KGFX_DESCRIPTOR_USAGE_UNIFORM_BUFFER = 1,
	KGFX_DESCRIPTOR_USAGE_STORAGE_BUFFER = 2,
	KGFX_DESCRIPTOR_USAGE_TEXTURE = 3,
	KGFX_DESCRIPTOR_USAGE_STORAGE_TEXTURE = 4,
	KGFX_DESCRIPTOR_USAGE_COUNT,
	KGFX_DESCRIPTOR_USAGE_MAX = KGFX_DESCRIPTOR_USAGE_COUNT - 1,
	KGFX_DESCRIPTOR_USAGE_MIN = KGFX_DESCRIPTOR_USAGE_UNIFORM_BUFFER,
} KGFXdescriptorusage;

typedef enum {
	KGFX_CULLMODE_NONE = 0,
	KGFX_CULLMODE_FRONT = 1,
	KGFX_CULLMODE_BACK = 2,
	KGFX_CULLMODE_COUNT,
	KGFX_CULLMODE_MAX = KGFX_CULLMODE_COUNT - 1,
	KGFX_CULLMODE_MIN = KGFX_CULLMODE_NONE,
} KGFXcullmode;

typedef enum {
	KGFX_FRONTFACE_CCW = 0,
	KGFX_FRONTFACE_CW = 1,
	KGFX_FRONTFACE_COUNT,
	KGFX_FRONTFACE_MAX = KGFX_FRONTFACE_COUNT - 1,
	KGFX_FRONTFACE_MIN = KGFX_FRONTFACE_CCW,
} KGFXfrontface;

typedef enum {
	KGFX_FILLMODE_SOLID = 0,
	KGFX_FILLMODE_LINES = 1,
	KGFX_FILLMODE_COUNT,
	KGFX_FILLMODE_MAX = KGFX_FILLMODE_COUNT - 1,
	KGFX_FILLMODE_MIN = KGFX_FILLMODE_SOLID,
} KGFXfillmode;

typedef enum {
	KGFX_TOPOLOGY_POINTS = 0,
	KGFX_TOPOLOGY_LINES = 1,
	KGFX_TOPOLOGY_TRIANGLES = 2,
	KGFX_TOPOLOGY_COUNT,
	KGFX_TOPOLOGY_MAX = KGFX_TOPOLOGY_COUNT - 1,
	KGFX_TOPOLOGY_MIN = KGFX_TOPOLOGY_POINTS,
} KGFXtopology;

/* buffer related enumerations */
typedef enum {
	KGFX_BUFFER_LOCATION_CPU = 0,
	KGFX_BUFFER_LOCATION_GPU = 1,
} KGFXbufferlocation;

typedef enum {
	KGFX_BUFFER_USAGE_VERTEX_BUFFER = 1,
	KGFX_BUFFER_USAGE_INDEX_BUFFER = 2,
	KGFX_BUFFER_USAGE_UNIFORM_BUFFER = 4,
	KGFX_BUFFER_USAGE_STORAGE_BUFFER = 8,
	KGFX_BUFFER_USAGE_TEXTURE_SRC = 16,
	KGFX_BUFFER_USAGE_COUNT,
	KGFX_BUFFER_USAGE_MAX = KGFX_BUFFER_USAGE_COUNT - 1,
	KGFX_BUFFER_USAGE_MIN = KGFX_BUFFER_USAGE_VERTEX_BUFFER,
} KGFXbufferusageflags;

/* texture related enumerations */
typedef enum {
	KGFX_TEXTURE_USAGE_SHADER_TEXTURE = 1,
	KGFX_TEXTURE_USAGE_RENDER_TARGET = 2,
} KGFXtextureusage;

typedef enum {
	/* 8-bit uint */
	KGFX_FORMAT_8_UINT = 1,
	KGFX_FORMAT_8_UINT2,
	KGFX_FORMAT_8_UINT4,

	/* 8-bit unorm */
	KGFX_FORMAT_8_UNORM,
	KGFX_FORMAT_8_UNORM2,
	KGFX_FORMAT_8_UNORM4,

	/* 8-bit unorm SRGB */
	KGFX_FORMAT_8_SRGBA,
	KGFX_FORMAT_8_SBGRA,

	/* 8-bit sint */
	KGFX_FORMAT_8_SINT,
	KGFX_FORMAT_8_SINT2,
	KGFX_FORMAT_8_SINT4,

	/* 8-bit snorm */
	KGFX_FORMAT_8_SNORM,
	KGFX_FORMAT_8_SNORM2,
	KGFX_FORMAT_8_SNORM4,

	/* 16-bit uint */
	KGFX_FORMAT_16_UINT,
	KGFX_FORMAT_16_UINT2,
	KGFX_FORMAT_16_UINT4,

	/* 16-bit unorm */
	KGFX_FORMAT_16_UNORM,
	KGFX_FORMAT_16_UNORM2,
	KGFX_FORMAT_16_UNORM4,

	/* 16-bit sint */
	KGFX_FORMAT_16_SINT,
	KGFX_FORMAT_16_SINT2,
	KGFX_FORMAT_16_SINT4,

	/* 16-bit snorm */
	KGFX_FORMAT_16_SNORM,
	KGFX_FORMAT_16_SNORM2,
	KGFX_FORMAT_16_SNORM4,

	/* 16-bit float */
	KGFX_FORMAT_16_FLOAT,
	KGFX_FORMAT_16_FLOAT2,
	KGFX_FORMAT_16_FLOAT4,

	/* 32-bit uint */
	KGFX_FORMAT_32_UINT,
	KGFX_FORMAT_32_UINT2,
	KGFX_FORMAT_32_UINT3,
	KGFX_FORMAT_32_UINT4,

	/* 32-bit sint */
	KGFX_FORMAT_32_SINT,
	KGFX_FORMAT_32_SINT2,
	KGFX_FORMAT_32_SINT3,
	KGFX_FORMAT_32_SINT4,

	/* 32-bit float */
	KGFX_FORMAT_32_FLOAT,
	KGFX_FORMAT_32_FLOAT2,
	KGFX_FORMAT_32_FLOAT3,
	KGFX_FORMAT_32_FLOAT4,

	/* depth */
	KGFX_FORMAT_32_FLOAT_DEPTH,
	KGFX_FORMAT_24_UNORM_DEPTH_8_UINT_STENCIL,
	KGFX_FORMAT_16_UNORM_DEPTH,
} KGFXformat;

typedef enum {
	KGFX_SAMPLER_FILTER_NEAREST = 0,
	KGFX_SAMPLER_FILTER_LINEAR = 1,
	KGFX_SAMPLER_FILTER_COUNT,
	KGFX_SAMPLER_FILTER_MAX = KGFX_SAMPLER_FILTER_COUNT - 1,
	KGFX_SAMPLER_FILTER_MIN = KGFX_SAMPLER_FILTER_NEAREST,
} KGFXsamplerfilter;

typedef enum {
	KGFX_SAMPLER_ADDRESS_MODE_REPEAT = 0,
	KGFX_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT = 1,
	KGFX_SAMPLER_ADDRESS_MODE_CLAMP = 2,
	KGFX_SAMPLER_ADDRESS_MODE_MIRRORED_CLAMP = 3,
	KGFX_SAMPLER_ADDRESS_MODE_COUNT,
	KGFX_SAMPLER_ADDRESS_MODE_MAX = KGFX_SAMPLER_ADDRESS_MODE_COUNT - 1,
	KGFX_SAMPLER_ADDRESS_MODE_MIN = KGFX_SAMPLER_ADDRESS_MODE_REPEAT,
} KGFXsampleraddressmode;

#endif
