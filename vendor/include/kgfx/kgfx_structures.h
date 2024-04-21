#ifndef KRISVERS_KGFX_STRUCTURES_H
#define KRISVERS_KGFX_STRUCTURES_H

#include <kgfx/kgfx_types.h>
#include <kgfx/kgfx_handles.h>
#include <kgfx/kgfx_enumerations.h>

/* pipeline related descriptors */
typedef struct {
	const char* entryName;
	const void* pData;
	u32 size;
	KGFXshadertype type;
	KGFXshadermedium medium;
} KGFXshaderdesc;

typedef struct {
	const char* semanticName;
	u32 semanticIndex;
	KGFXdatatype type;
	u32 location;
} KGFXpipelineattribute;

typedef struct {
	KGFXinputrate inputRate;
	KGFXpipelineattribute* pAttributes;
	u32 attributeCount;
	KGFXbindpoint bindpoint;
	u32 binding;
} KGFXpipelinebinding;

typedef struct {
	KGFXbindpoint bindpoint;
	u32 binding;
	KGFXdescriptorusage usage;
	u32 size;
} KGFXdescriptorsetdesc;

typedef struct {
	KGFXpipelinebinding* pBindings;
	u32 bindingCount;
	KGFXdescriptorsetdesc* pDescriptorSets;
	u32 descriptorSetCount;
} KGFXpipelinelayout;

typedef struct {
	KGFXshader* pShaders;
	u32 shaderCount;
	KGFXframebuffer framebuffer;
	KGFXpipelinelayout layout;
	KGFXcullmode cullMode;
	KGFXfrontface frontFace;
	KGFXfillmode fillMode;
	KGFXtopology topology;
} KGFXpipelinedesc;

/* buffer related structures */
typedef struct {
	KGFXbufferlocation location;
	KGFXbufferusageflags usage;
	u32 size;
	void* pData;
} KGFXbufferdesc;

/* texture related structures */
typedef struct {
	KGFXformat format;
	KGFXtextureusage usage;
	u32 width;
	u32 height;
	u32 depth;
} KGFXtexturedesc;

typedef struct {
	KGFXsamplerfilter magFilter;
	KGFXsamplerfilter minFilter;
	KGFXsamplerfilter mipmapMode;
	KGFXsampleraddressmode addressModeU;
	KGFXsampleraddressmode addressModeV;
	KGFXsampleraddressmode addressModeW;
} KGFXsamplerdesc;

/* framebuffer */
typedef struct {
	KGFXtexture* pTextures;
	u32 textureCount;
} KGFXframebufferdesc;

#endif