
#include "Hooks.h"

namespace {
void messageHandler(SKSE::MessagingInterface::Message* message)
{
    switch (message->type) {
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

// BUG: Second window breaks keyboard input
// BUG: Steam overlay stack overflow
// TODO: NG RE