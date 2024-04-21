#ifndef KRISVERS_KGFX_H
#define KRISVERS_KGFX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <kgfx/kgfx_types.h>
#include <kgfx/kgfx_platform.h>
#include <kgfx/kgfx_handles.h>
#include <kgfx/kgfx_structures.h>
#include <kgfx/kgfx_enumerations.h>
	
/*
	KGFX versions (32 bit)

	2 nibbles for major
	3 nibbles for minor
	3 nibbles for patch
	1.2.3 = 0x01002003
*/
#define KGFX_MAKE_VERSION(major, minor, patch) ((u32) (((major & 0xFF) << 24) | ((minor & 0xFFF) << 12) | (patch & 0xFFF)))
#define KGFX_HEADER_VERSION 0x01000000
#define KGFX_ANY_MAJOR 0xFF
#define KGFX_ANY_MINOR 0xFFF
#define KGFX_ANY_PATCH 0xFFF
#define KGFX_MAJOR(version) ((version >> 24) & 0xFF)
#define KGFX_MINOR(version) ((version >> 12) & 0xFFF)
#define KGFX_PATCH(version) (version & 0xFFF)
#define KGFX_ANY_VERSION KGFX_MAKE_VERSION(KGFX_ANY_MAJOR, KGFX_ANY_MINOR, KGFX_ANY_PATCH)
#define KGFX_MAGIC 0x8BF7E6A8
#define KGFX_IS_SUCCESSFUL(result) ((result) == KGFX_SUCCESS)

#ifndef KGFX_API
#define KGFX_API
#endif

/*
	initializes a kgfx context with requested version
	any non KGFX_SUCCESS result is a failure

	the requested version is the oldest version the application requests
	future versions are compatible with the previous versions when used properly

	if the requested version is more recent than the supported version, this function will return a KGFX_VERSION_NOT_SUPPORTED error
	if the requested version is older than the supported version, this function will return a context with the implementation version
		the version is not guaranteed to be the requested version, but it will be equal or higher

	if the requested version has any of the major, minor or patch set to KGFX_ANY_MAJOR, KGFX_ANY_MINOR or KGFX_ANY_PATCH, the version that is equal or higher will be returned
*/
KGFX_API KGFXresult kgfxCreateContext(u32 version, KGFXwindow window, KGFXcontext* context);

/* destroys a kgfx context */
KGFX_API void kgfxDestroyContext(KGFXcontext ctx);

KGFX_API KGFXshader kgfxCreateShader(KGFXcontext ctx, KGFXshaderdesc shaderDesc);

KGFX_API void kgfxDestroyShader(KGFXcontext ctx, KGFXshader shader);

KGFX_API KGFXpipeline kgfxCreatePipeline(KGFXcontext ctx, KGFXpipelinedesc pipelineDesc);

KGFX_API void kgfxDestroyPipeline(KGFXcontext ctx, KGFXpipeline pipeline);

KGFX_API KGFXbuffer kgfxCreateBuffer(KGFXcontext ctx, KGFXbufferdesc bufferDesc);

KGFX_API KGFXresult kgfxBufferUpload(KGFXcontext ctx, KGFXbuffer buffer, u32 size, void* data);

KGFX_API void* kgfxBufferMap(KGFXcontext ctx, KGFXbuffer buffer);

KGFX_API void kgfxBufferUnmap(KGFXcontext ctx, KGFXbuffer buffer);

KGFX_API KGFXresult kgfxBufferCopy(KGFXcontext ctx, KGFXbuffer dstBuffer, KGFXbuffer srcBuffer, u32 size, u32 dstOffset, u32 srcOffset);

KGFX_API u32 kgfxBufferSize(KGFXcontext ctx, KGFXbuffer buffer);

KGFX_API void kgfxDestroyBuffer(KGFXcontext ctx, KGFXbuffer buffer);

KGFX_API KGFXtexture kgfxCreateTexture(KGFXcontext ctx, KGFXtexturedesc textureDesc);

KGFX_API KGFXresult kgfxCopyBufferToTexture(KGFXcontext ctx, KGFXtexture dstTexture, KGFXbuffer srcBuffer, u32 srcOffset);

KGFX_API void kgfxDestroyTexture(KGFXcontext ctx, KGFXtexture texture);

KGFX_API KGFXsampler kgfxCreateSampler(KGFXcontext ctx, KGFXsamplerdesc samplerDesc);

KGFX_API void kgfxDestroySampler(KGFXcontext ctx, KGFXsampler sampler);

KGFX_API KGFXframebuffer kgfxCreateFramebuffer(KGFXcontext ctx, KGFXframebufferdesc framebufferDesc);

KGFX_API void kgfxDestroyFramebuffer(KGFXcontext ctx, KGFXframebuffer framebuffer);

KGFX_API KGFXcommandlist kgfxCreateCommandList(KGFXcontext ctx);

KGFX_API void kgfxCommandReset(KGFXcontext ctx, KGFXcommandlist commandList);

KGFX_API void kgfxCommandBindPipeline(KGFXcontext ctx, KGFXcommandlist commandList, KGFXpipeline pipeline);

KGFX_API void kgfxCommandBindVertexBuffer(KGFXcontext ctx, KGFXcommandlist commandList, KGFXbuffer buffer, u32 binding);

KGFX_API void kgfxCommandBindIndexBuffer(KGFXcontext ctx, KGFXcommandlist commandList, KGFXbuffer buffer, u32 binding);

KGFX_API void kgfxCommandBindDescriptorSetBuffer(KGFXcontext ctx, KGFXcommandlist commandList, KGFXbuffer buffer, u32 binding);

KGFX_API void kgfxCommandBindDescriptorSetTexture(KGFXcontext ctx, KGFXcommandlist commandList, KGFXtexture texture, KGFXsampler sampler, u32 binding);

KGFX_API void kgfxCommandDraw(KGFXcontext ctx, KGFXcommandlist commandList, u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance);

KGFX_API void kgfxCommandDrawIndexed(KGFXcontext ctx, KGFXcommandlist commandList, u32 indexCount, u32 instanceCount, u32 firstIndex, s32 vertexOffset, u32 firstInstance);

KGFX_API void kgfxCommandListSubmit(KGFXcontext ctx, KGFXcommandlist commandList);

KGFX_API void kgfxPresent(KGFXcontext ctx);

KGFX_API void kgfxDestroyCommandList(KGFXcontext ctx, KGFXcommandlist commandList);

/* returns implementation version */
KGFX_API u32 kgfxGetImplementationVersion(void);

/* if pBackends is null, returns the number of supported backends. otherwise it returns the number written to pBackends (less than or equal to count) */
KGFX_API u32 kgfxGetBackends(KGFXbackend* pBackends, u32 count);

#ifdef __cplusplus
}
#endif

#endif
