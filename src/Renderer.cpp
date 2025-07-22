#include "Renderer.h"

#include "Globals.h"

namespace {
// ============================================================================================================================
LRESULT CALLBACK windowProcessFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ============================================================================================================================
HWND createSecondWindow(
    const RE::BSGraphics::RendererInitOSData&          os_data,
    const RE::BSGraphics::ApplicationWindowProperties& props)
{
    HWND      main_wnd  = reinterpret_cast<HWND>(os_data.hwnd);
    HINSTANCE main_inst = reinterpret_cast<HINSTANCE>(os_data.instance);

    WNDCLASSEX wnd_class;
    ZeroMemory(&wnd_class, sizeof(WNDCLASSEX));
    wnd_class.cbClsExtra    = NULL;
    wnd_class.cbSize        = sizeof(WNDCLASSEX);
    wnd_class.cbWndExtra    = NULL;
    wnd_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
    wnd_class.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wnd_class.hIcon         = NULL;
    wnd_class.hIconSm       = NULL;
    wnd_class.hInstance     = main_inst;
    wnd_class.lpfnWndProc   = windowProcessFn; // reinterpret_cast<WNDPROC>(os_data.windowProcFunction);
    wnd_class.lpszClassName = L"d3d12 window";
    wnd_class.lpszMenuName  = NULL;
    wnd_class.style         = CS_HREDRAW | CS_VREDRAW;

    if (!RegisterClassEx(&wnd_class)) {
        auto result = GetLastError();
        MessageBox(NULL,
                   std::format(L"Window class creation failed.\nError: {}", result).data(),
                   L"Window Class Failed",
                   MB_ICONERROR);
        return NULL;
    }

    HWND hwnd = CreateWindowEx(NULL,
                               wnd_class.lpszClassName,
                               L"D3D12 Window",
                               WS_OVERLAPPEDWINDOW,
                               props.windowX,
                               props.windowY,
                               props.screenSize.width,
                               props.screenSize.height,
                               main_wnd,
                               NULL,
                               main_inst,
                               NULL);

    if (hwnd == nullptr) {
        auto result = GetLastError();
        MessageBox(NULL,
                   std::format(L"Window creation failed.\nError: {}", result).data(),
                   L"Window Creation Failed",
                   MB_ICONERROR);
        return NULL;
    }

    ShowWindow(hwnd, SW_SHOWNORMAL);

    Logger::info("Window creation succeeded.");

    return hwnd;
}

// ============================================================================================================================
// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
void getHardwareAdapter(
    IDXGIFactory1*  pFactory,
    IDXGIAdapter1** ppAdapter,
    bool            requestHighPerformanceAdapter = false)
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
        for (
            UINT adapter_index = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapter_index,
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
            ++adapter_index) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0U) {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr) {
        for (UINT adapter_index = 0; SUCCEEDED(pFactory->EnumAdapters1(adapter_index, &adapter)); ++adapter_index) {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0U) {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}
} // namespace


// ============================================================================================================================
void Renderer::onRendererInit(
    RE::BSGraphics::RendererInitOSData*          a_data,
    RE::BSGraphics::ApplicationWindowProperties* a_windowProps)
{
    main_window_data = *a_data;
    window_props     = *a_windowProps;

    Logger::info("RE::BSGraphics::ApplicationWindowProperties:\n\t"
                 "screenSize = {}, {}\n\t"
                 "windowX = {}\n\t"
                 "windowY = {}\n\t"
                 "refreshRate = {}\n\t"
                 "presentInterval = {}\n\t"
                 "appFullScreen = {}\n\t"
                 "borderlessWindow = {}\n\t"
                 "vsync = {}",
                 window_props.screenSize.width,
                 window_props.screenSize.height,
                 window_props.windowX,
                 window_props.windowY,
                 window_props.refreshRate,
                 window_props.presentInterval,
                 window_props.appFullScreen,
                 window_props.borderlessWindow,
                 window_props.vsync);

    window = createSecondWindow(main_window_data, window_props);

    if (window != nullptr)
        initD3d();
}

// ============================================================================================================================
void Renderer::initD3d()
{

    UINT                  dxgi_factory_flags = 0;
    ComPtr<IDXGIFactory4> factory;
    DX::ThrowIfFailed(CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&factory)));

    ComPtr<IDXGIAdapter1> hardware_adapter;
    getHardwareAdapter(factory.Get(), &hardware_adapter);

    DX::ThrowIfFailed(D3D12CreateDevice(
        hardware_adapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&device)));

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queue_desc = {};
    queue_desc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;

    DX::ThrowIfFailed(device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&cmd_queue)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {};
    swapchain_desc.BufferCount           = FRAME_COUNT;
    swapchain_desc.Width                 = window_props.screenSize.width;
    swapchain_desc.Height                = window_props.screenSize.height;
    swapchain_desc.Format                = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchain_desc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchain_desc.SampleDesc.Count      = 1;

    ComPtr<IDXGISwapChain1> swapchain1;
    DX::ThrowIfFailed(factory->CreateSwapChainForHwnd(
        cmd_queue.Get(), // Swap chain needs the queue so that it can force a flush on it.
        window,
        &swapchain_desc,
        nullptr,
        nullptr,
        &swapchain1));

    // This sample does not support fullscreen transitions.
    DX::ThrowIfFailed(factory->MakeWindowAssociation(window, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER));

    DX::ThrowIfFailed(swapchain1.As(&swapchain));
    frame_index = swapchain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
        rtv_heap_desc.NumDescriptors             = FRAME_COUNT;
        rtv_heap_desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        DX::ThrowIfFailed(device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap)));

        rtv_descriptor_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_heap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT i = 0; i < FRAME_COUNT; i++) {
            DX::ThrowIfFailed(swapchain->GetBuffer(i, IID_PPV_ARGS(&swapchain_rts[i])));
            device->CreateRenderTargetView(swapchain_rts[i].Get(), nullptr, rtv_handle);
            rtv_handle.Offset(1, rtv_descriptor_size);
        }
    }

    DX::ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmd_allocator)));

    // Create the command list.
    DX::ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmd_allocator.Get(), nullptr, IID_PPV_ARGS(&cmd_list)));

    // Command lists are created in the recording state, but there is nothing
    // to record yet. The main loop expects it to be closed, so close it now.
    DX::ThrowIfFailed(cmd_list->Close());

    // Create synchronization objects.
    {
        DX::ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
        fence_value = 1;

        // Create an event handle to use for frame synchronization.
        fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (fence_event == nullptr) {
            DX::ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

    Logger::info("D3D12 initialized.");
}

// ============================================================================================================================
void Renderer::draw()
{
    {
        // Command list allocators can only be reset when the associated
        // command lists have finished execution on the GPU; apps should use
        // fences to determine GPU execution progress.
        DX::ThrowIfFailed(cmd_allocator->Reset());

        // However, when ExecuteCommandList() is called on a particular command
        // list, that command list can then be reset at any time and must be before
        // re-recording.
        DX::ThrowIfFailed(cmd_list->Reset(cmd_allocator.Get(), pipeline_state.Get()));

        // Indicate that the back buffer will be used as a render target.
        auto transition = CD3DX12_RESOURCE_BARRIER::Transition(swapchain_rts[frame_index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        cmd_list->ResourceBarrier(1, &transition);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(rtv_heap->GetCPUDescriptorHandleForHeapStart(), frame_index, rtv_descriptor_size);

        // Record commands.
        const float clear_color[] = {0.F, 0.2F, 0.4F, 1.0F};
        cmd_list->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);

        // Indicate that the back buffer will now be used to present.
        transition = CD3DX12_RESOURCE_BARRIER::Transition(swapchain_rts[frame_index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        cmd_list->ResourceBarrier(1, &transition);

        DX::ThrowIfFailed(cmd_list->Close());
    }

    // Execute the command list.
    ID3D12CommandList* cmd_lists[] = {cmd_list.Get()};
    cmd_queue->ExecuteCommandLists(_countof(cmd_lists), cmd_lists);

    // Present the frame.
    DX::ThrowIfFailed(swapchain->Present(1, 0));

    {
        // FIXME
        // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
        // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
        // sample illustrates how to use fences for efficient resource usage and to
        // maximize GPU utilization.

        // Signal and increment the fence value.
        const UINT64 old_fence_value = fence_value;
        DX::ThrowIfFailed(cmd_queue->Signal(fence.Get(), old_fence_value));
        fence_value++;

        // Wait until the previous frame is finished.
        if (fence->GetCompletedValue() < old_fence_value) {
            DX::ThrowIfFailed(fence->SetEventOnCompletion(old_fence_value, fence_event));
            WaitForSingleObject(fence_event, INFINITE);
        }

        frame_index = swapchain->GetCurrentBackBufferIndex();
    }
}