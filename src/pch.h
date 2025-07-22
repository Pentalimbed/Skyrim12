#pragma once

#include <cmath>
#include <cstdint>
#include <format>

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <d3d11.h>
#include <d3d12.h>
#include <d3dx12.h>

namespace DX {
// Helper class for COM exceptions
class com_exception : public std::exception {
public:
    explicit com_exception(HRESULT hr) noexcept :
        result(hr) {}

    const char* what() const override
    {
        static char s_str[64] = {};
        sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
        return s_str;
    }

private:
    HRESULT result;
};

// Helper utility converts D3D API failures into exceptions.
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr)) {
        throw com_exception(hr);
    }
}
} // namespace DX

namespace Logger = SKSE::log;
using namespace std::literals;
