#pragma once

#include <array>
#include <cstdint>

struct Renderer {
    RE::BSGraphics::RendererInitOSData          main_window_data = {};
    RE::BSGraphics::ApplicationWindowProperties window_props     = {};
    HWND                                        window           = nullptr;

    constexpr static uint32_t                       FRAME_COUNT         = 2;
    ComPtr<IDXGISwapChain3>                         swapchain           = nullptr;
    ComPtr<ID3D12Device>                            device              = nullptr;
    std::array<ComPtr<ID3D12Resource>, FRAME_COUNT> swapchain_rts       = {nullptr};
    ComPtr<ID3D12CommandAllocator>                  cmd_allocator       = nullptr;
    ComPtr<ID3D12CommandQueue>                      cmd_queue           = nullptr;
    ComPtr<ID3D12DescriptorHeap>                    rtv_heap            = nullptr;
    ComPtr<ID3D12PipelineState>                     pipeline_state      = nullptr;
    ComPtr<ID3D12GraphicsCommandList>               cmd_list            = nullptr;
    UINT                                            rtv_descriptor_size = 0;

    UINT                frame_index = 0;
    HANDLE              fence_event = nullptr;
    ComPtr<ID3D12Fence> fence       = nullptr;
    UINT64              fence_value = UINT64_MAX;

    void onRendererInit(
        RE::BSGraphics::RendererInitOSData*          a_data,
        RE::BSGraphics::ApplicationWindowProperties* a_windowProps,
        REX::W32::HWND                               a_window);
    void initD3d();

    void draw();
};
