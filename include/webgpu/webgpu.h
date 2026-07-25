#pragma once
#include <stdint.h>

// Minimal wgpu-native type definitions for Dear ImGui backend.
typedef struct WGPUDeviceImpl*      WGPUDevice;
typedef struct WGPUQueueImpl*       WGPUQueue;
typedef struct WGPUTextureViewImpl* WGPUTextureView;
typedef struct WGPUCommandEncoderImpl*   WGPUCommandEncoder;
typedef struct WGPURenderPassEncoderImpl* WGPURenderPassEncoder;
typedef struct WGPUCommandBufferImpl*    WGPUCommandBuffer;

typedef enum WGPUTextureFormat {
    WGPUTextureFormat_Undefined        = 0,
    WGPUTextureFormat_RGBA8Unorm       = 1,
    WGPUTextureFormat_RGBA8UnormSrgb   = 2,
    WGPUTextureFormat_BGRA8Unorm       = 3,
    WGPUTextureFormat_BGRA8UnormSrgb   = 4,
} WGPUTextureFormat;

typedef enum WGPULoadOp {
    WGPULoadOp_Load  = 0,
    WGPULoadOp_Clear = 1,
} WGPULoadOp;

typedef enum WGPUStoreOp {
    WGPUStoreOp_Store = 0,
    WGPUStoreOp_Discard = 1,
} WGPUStoreOp;

typedef struct WGPUColor {
    double r, g, b, a;
} WGPUColor;

typedef struct WGPURenderPassColorAttachment {
    WGPUTextureView view;
    WGPUTextureView resolve_target;
    void*           depth_slice;
    WGPULoadOp      load_op;
    WGPUStoreOp     store_op;
    WGPUColor       clear_value;
} WGPURenderPassColorAttachment;

typedef struct WGPURenderPassDepthStencilAttachment {
    WGPUTextureView view;
    WGPULoadOp      depth_load_op;
    WGPUStoreOp     depth_store_op;
    float           depth_clear_value;
    uint8_t         depth_read_only;
    WGPULoadOp      stencil_load_op;
    WGPUStoreOp     stencil_store_op;
    uint32_t        stencil_clear_value;
    uint8_t         stencil_read_only;
} WGPURenderPassDepthStencilAttachment;

typedef struct WGPURenderPassDescriptor {
    void*                             next_in_chain;
    const char*                       label;
    uint32_t                          color_attachment_count;
    const WGPURenderPassColorAttachment* color_attachments;
    const WGPURenderPassDepthStencilAttachment* depth_stencil_attachment;
    uint32_t                          timestamp_write_count;
    void*                             timestamp_writes;
    uint32_t                          occlusion_query_set_count;
    void*                             occlusion_query_sets;
} WGPURenderPassDescriptor;

WGPUCommandEncoder wgpuDeviceCreateCommandEncoder(WGPUDevice device, void* desc);
WGPURenderPassEncoder wgpuRenderPassEncoderBegin(WGPUCommandEncoder enc, const WGPURenderPassDescriptor* desc);
void wgpuRenderPassEncoderEnd(WGPURenderPassEncoder pass);
void wgpuRenderPassEncoderSetPipeline(WGPURenderPassEncoder pass, void* pipeline);
void wgpuRenderPassEncoderSetScissorRect(WGPURenderPassEncoder pass, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
void wgpuRenderPassEncoderDraw(WGPURenderPassEncoder pass, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
WGPUCommandBuffer wgpuCommandEncoderFinish(WGPUCommandEncoder enc, void* desc);
void wgpuQueueSubmit(WGPUQueue queue, uint32_t count, const WGPUCommandBuffer* cmds);
