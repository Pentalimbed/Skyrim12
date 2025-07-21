#include "Hooks.h"

#include "Globals.h"
#include "RE/R/Renderer.h"
#include "Renderer.h"

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
    *(uintptr_t*)&T::func = Detours::X64::DetourClassVTable(*static_cast<uintptr_t*>(target), &T::thunk, idx);
}
} // namespace stl
} // namespace

// ============================================================================================================================
namespace Hooks {
// TODO: find a better place to hook
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
struct BSGraphics_Renderer_Init {
    static void thunk(
        RE::BSGraphics::Renderer*                    a_this,
        RE::BSGraphics::RendererInitOSData*          a_data,
        RE::BSGraphics::ApplicationWindowProperties* a_windowProps,
        REX::W32::HWND                               a_window)
    {
        func(a_this, a_data, a_windowProps, a_window);
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

} // namespace Hooks