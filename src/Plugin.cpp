
#include "Hooks.h"

namespace {
void messageHandler(SKSE::MessagingInterface::Message* message)
{
    switch (message->type) {
        case SKSE::MessagingInterface::kPostPostLoad: {
            Hooks::installEventListeners();
            break;
        }
    }
}
} // namespace

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    const auto* messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener("SKSE", messageHandler);

    Hooks::installHooks();

    return true;
}

// TODO: NG RE
// BUG: Steam overlay stack overflow