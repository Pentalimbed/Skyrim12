
#include "Hooks.h"

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    Hooks::installHooks();

    return true;
}


// BUG: Steam overlay stack overflow