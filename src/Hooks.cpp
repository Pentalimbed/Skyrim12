#include "Hooks.h"

#include "Globals.h"
#include "Renderer.h"

#include <stacktrace>
#include <detours/Detours.h>

// Disable linting because of stl/hook naming convention
// NOLINTBEGIN(readability-identifier-naming)

// No littering hooks everywhere!
namespace {
namespace stl {
using namespace SKSE::stl;

template <class T, std::size_t Size = 5>
void write_thunk_call(std::uintptr_t a_src)
{
    SKSE::AllocTrampoline(14);
    auto& trampoline = SKSE::GetTrampoline();
    if (Size == 6) {
        T::func = *(uintptr_t*)trampoline.write_call<6>(a_src, T::thunk);
    } else {
        T::func = trampoline.write_call<Size>(a_src, T::thunk);
    }
}

template <class F, size_t index, class T>
void write_vfunc()
{
    REL::Relocation<std::uintptr_t> vtbl{F::VTABLE[index]};
    T::func = vtbl.write_vfunc(T::size, T::thunk);
}

template <std::size_t idx, class T>
void write_vfunc(REL::VariantID id)
{
    REL::Relocation<std::uintptr_t> vtbl{id};
    T::func = vtbl.write_vfunc(idx, T::thunk);
}

template <class T>
void write_thunk_jmp(std::uintptr_t a_src)
{
    SKSE::AllocTrampoline(14);
    auto& trampoline = SKSE::GetTrampoline();
    T::func          = trampoline.write_branch<5>(a_src, T::thunk);
}

template <class F, class T>
void write_vfunc()
{
    write_vfunc<F, 0, T>();
}

template <class T>
void detour_thunk(REL::RelocationID a_relId)
{
    *(uintptr_t*)&T::func = Detours::X64::DetourFunction(a_relId.address(), (uintptr_t)&T::thunk);
}

template <class T>
void detour_thunk_ignore_func(REL::RelocationID a_relId)
{
    std::ignore = Detours::X64::DetourFunction(a_relId.address(), (uintptr_t)&T::thunk);
}

template <std::size_t idx, class T>
void detour_vfunc(void* target)
{
    *(uintptr_t*)&T::func = Detours::X64::DetourClassVTable(*(uintptr_t*)target, &T::thunk, idx);
}
} // namespace stl
} // namespace

namespace Hooks {
// ============================================================================================================================
// HACK: Debugging call stack of every draw/dispatch
struct ID3D11DeviceContext_StackTracer {
    static inline bool doLog = false;
    struct DebugEventSink : public RE::BSTEventSink<RE::InputEvent> {
        RE::BSEventNotifyControl ProcessEvent(const RE::InputEvent* a_event, [[maybe_unused]] RE::BSTEventSource<RE::InputEvent>* a_eventSource) override
        {
            if ((a_event != nullptr) && (a_eventSource != nullptr) && (a_event->eventType == RE::INPUT_EVENT_TYPE::kChar)) {
                const auto* event = a_event->AsCharEvent();
                if (event->keyCode == '[')
                    doLog = !doLog;
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };
    static inline DebugEventSink sink = {};

    static void printStackTrace()
    {
        if (doLog)
            Logger::info("{}", std::stacktrace::current());
    }

    static void thunkDrawIndexed(
        ID3D11DeviceContext* a_this,
        UINT                 IndexCount,
        UINT                 StartIndexLocation,
        INT                  BaseVertexLocation)
    {
        printStackTrace();
        funcDrawIndexed(a_this, IndexCount, StartIndexLocation, BaseVertexLocation);
    }
    static inline REL::Relocation<decltype(thunkDrawIndexed)> funcDrawIndexed;

    static void thunkDraw(
        ID3D11DeviceContext* a_this,
        UINT                 VertexCount,
        UINT                 StartVertexLocation)
    {
        printStackTrace();
        funcDraw(a_this, VertexCount, StartVertexLocation);
    }
    static inline REL::Relocation<decltype(thunkDraw)> funcDraw;

    static void thunkDrawIndexedInstanced(
        ID3D11DeviceContext* a_this,
        UINT                 IndexCountPerInstance,
        UINT                 InstanceCount,
        UINT                 StartIndexLocation,
        INT                  BaseVertexLocation,
        UINT                 StartInstanceLocation)
    {
        printStackTrace();
        funcDrawIndexedInstanced(a_this, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
    }
    static inline REL::Relocation<decltype(thunkDrawIndexedInstanced)> funcDrawIndexedInstanced;

    static void thunkDrawInstanced(
        ID3D11DeviceContext* a_this,
        UINT                 VertexCountPerInstance,
        UINT                 InstanceCount,
        UINT                 StartVertexLocation,
        UINT                 StartInstanceLocation)
    {
        printStackTrace();
        funcDrawInstanced(a_this, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
    }
    static inline REL::Relocation<decltype(thunkDrawInstanced)> funcDrawInstanced;

    static void thunkDrawIndexedInstancedIndirect(
        ID3D11DeviceContext* a_this,
        ID3D11Buffer*        pBufferForArgs,
        UINT                 AlignedByteOffsetForArgs)
    {
        printStackTrace();
        funcDrawIndexedInstancedIndirect(a_this, pBufferForArgs, AlignedByteOffsetForArgs);
    }
    static inline REL::Relocation<decltype(thunkDrawIndexedInstancedIndirect)> funcDrawIndexedInstancedIndirect;

    static void thunkDrawInstancedIndirect(
        ID3D11DeviceContext* a_this,
        ID3D11Buffer*        pBufferForArgs,
        UINT                 AlignedByteOffsetForArgs)
    {
        printStackTrace();
        funcDrawInstancedIndirect(a_this, pBufferForArgs, AlignedByteOffsetForArgs);
    }
    static inline REL::Relocation<decltype(thunkDrawInstancedIndirect)> funcDrawInstancedIndirect;

    static void thunkDispatch(
        ID3D11DeviceContext* a_this,
        UINT                 ThreadGroupCountX,
        UINT                 ThreadGroupCountY,
        UINT                 ThreadGroupCountZ)
    {
        printStackTrace();
        funcDispatch(a_this, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
    }
    static inline REL::Relocation<decltype(thunkDispatch)> funcDispatch;

    static void thunkDispatchIndirect(
        ID3D11DeviceContext* a_this,
        ID3D11Buffer*        pBufferForArgs,
        UINT                 AlignedByteOffsetForArgs)
    {
        printStackTrace();
        funcDispatchIndirect(a_this, pBufferForArgs, AlignedByteOffsetForArgs);
    }
    static inline REL::Relocation<decltype(thunkDispatchIndirect)> funcDispatchIndirect;

    static void install()
    {
        constexpr size_t OFFSET_VTABLE = 3 + 4; // IUnknown + IDeviceChild
        auto*            context       = reinterpret_cast<ID3D11DeviceContext*>(RE::BSGraphics::Renderer::GetSingleton()->GetRuntimeData().context);

        *(uintptr_t*)(&funcDrawIndexed)                  = Detours::X64::DetourClassVTable(*(uintptr_t*)context, &thunkDrawIndexed, OFFSET_VTABLE + 5);
        *(uintptr_t*)(&funcDraw)                         = Detours::X64::DetourClassVTable(*(uintptr_t*)context, &thunkDraw, OFFSET_VTABLE + 6);
        *(uintptr_t*)(&funcDrawIndexedInstanced)         = Detours::X64::DetourClassVTable(*(uintptr_t*)context, &thunkDrawIndexedInstanced, OFFSET_VTABLE + 13);
        *(uintptr_t*)(&funcDrawInstanced)                = Detours::X64::DetourClassVTable(*(uintptr_t*)context, &thunkDrawInstanced, OFFSET_VTABLE + 14);
        *(uintptr_t*)(&funcDrawIndexedInstancedIndirect) = Detours::X64::DetourClassVTable(*(uintptr_t*)(context), &thunkDrawIndexedInstancedIndirect, OFFSET_VTABLE + 32);
        *(uintptr_t*)(&funcDrawInstancedIndirect)        = Detours::X64::DetourClassVTable(*(uintptr_t*)(context), &thunkDrawInstancedIndirect, OFFSET_VTABLE + 33);
        *(uintptr_t*)(&funcDispatch)                     = Detours::X64::DetourClassVTable(*(uintptr_t*)(context), &thunkDispatch, OFFSET_VTABLE + 34);
        *(uintptr_t*)(&funcDispatchIndirect)             = Detours::X64::DetourClassVTable(*(uintptr_t*)(context), &thunkDispatchIndirect, OFFSET_VTABLE + 35);
    }

    static void installEventListeners()
    {
        RE::BSInputDeviceManager::GetSingleton()->AddEventSink(&ID3D11DeviceContext_StackTracer::sink);
    }
};

// ============================================================================================================================
// All the rendering and present
// TODO: Find a better place to hook
struct BSGraphics_Renderer_Begin_sub_140D74B40 {
    static uint64_t WINAPI thunk(uint64_t a1, uint64_t a2)
    {
        auto retval = func(a1, a2);
        Globals::renderer.draw();
        return retval;
    }
    static inline REL::Relocation<decltype(thunk)> func;

    static void install() { stl::write_thunk_call<BSGraphics_Renderer_Begin_sub_140D74B40>(REL::ID(75460).address() + 0x1d3); }
};

// ============================================================================================================================
// Creates window and d3d12 backend
struct BSGraphics_Renderer_Init {
    static void thunk(
        RE::BSGraphics::Renderer*                    a_this,
        RE::BSGraphics::RendererInitOSData*          a_data,
        RE::BSGraphics::ApplicationWindowProperties* a_windowProps,
        REX::W32::HWND                               a_window)
    {
        func(a_this, a_data, a_windowProps, a_window);
        ID3D11DeviceContext_StackTracer::install();
        Globals::renderer.onRendererInit(a_data, a_windowProps);
    }
    static inline REL::Relocation<decltype(thunk)> func;

    static void install() { stl::write_thunk_call<BSGraphics_Renderer_Init>(REL::ID(35548).address() + 0x8a0); }
};

// NOLINTEND(readability-identifier-naming)

// ============================================================================================================================
void installHooks()
{
    BSGraphics_Renderer_Init::install();
    BSGraphics_Renderer_Begin_sub_140D74B40::install();
}

// ============================================================================================================================
void installEventListeners()
{
    ID3D11DeviceContext_StackTracer::installEventListeners();
}
} // namespace Hooks