
#include "Hooks.h"

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    Hooks::installHooks();

    return true;
}

// TODO: NG RE
// BUG: Steam overlay stack overflow