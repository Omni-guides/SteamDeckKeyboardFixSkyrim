// Copyright (C) 2026 Omni-Guides
// SPDX-License-Identifier: GPL-3.0-only

#include <SKSE/SKSE.h>
#include <MinHook.h>
#include <Windows.h>
#include <atomic>
#include <memory>
#include <string_view>
#include <spdlog/sinks/basic_file_sink.h>

using namespace std::literals;

namespace logger = SKSE::log;

namespace
{
    constexpr ULONG_PTR kExecuteAccess = 8;

    using ReleaseCurrentThreadMemory = void(__stdcall*)();
    using RunCallbacks = void(__stdcall*)();

    ReleaseCurrentThreadMemory g_releaseCurrentThreadMemory = nullptr;
    RunCallbacks g_originalRunCallbacks = nullptr;
    std::atomic_flag g_exceptionLogged;

    bool IsRunningUnderWine() noexcept
    {
        const auto ntdll = GetModuleHandleW(L"ntdll.dll");
        return ntdll && GetProcAddress(ntdll, "wine_get_version");
    }

    LONG FilterNullExecute(const EXCEPTION_POINTERS* exception) noexcept
    {
        const auto* record = exception ? exception->ExceptionRecord : nullptr;
        if (!record ||
            record->ExceptionCode != EXCEPTION_ACCESS_VIOLATION ||
            record->ExceptionAddress != nullptr ||
            record->NumberParameters < 2 ||
            record->ExceptionInformation[0] != kExecuteAccess ||
            record->ExceptionInformation[1] != 0) {
            return EXCEPTION_CONTINUE_SEARCH;
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }

    // Keep SEH separate from functions that contain C++ objects requiring unwinding.
    bool RunCallbacksGuarded() noexcept
    {
        __try {
            g_originalRunCallbacks();
            return false;
        } __except (FilterNullExecute(GetExceptionInformation())) {
            return true;
        }
    }

    void __stdcall RunCallbacksHook()
    {
        if (RunCallbacksGuarded()) {
            if (!g_exceptionLogged.test_and_set()) {
                logger::warn("Contained a null execute violation from SteamAPI_RunCallbacks.");
            }
            g_releaseCurrentThreadMemory();
        }
    }

    void InstallHook()
    {
        if (!IsRunningUnderWine()) {
            logger::info("Native Windows detected; hook not installed.");
            return;
        }

        const auto steamApi = GetModuleHandleW(L"steam_api64.dll");
        if (!steamApi) {
            logger::error("steam_api64.dll is not loaded.");
            return;
        }

        g_releaseCurrentThreadMemory = reinterpret_cast<ReleaseCurrentThreadMemory>(
            GetProcAddress(steamApi, "SteamAPI_ReleaseCurrentThreadMemory"));
        if (!g_releaseCurrentThreadMemory) {
            logger::error("SteamAPI_ReleaseCurrentThreadMemory was not found.");
            return;
        }

        auto status = MH_Initialize();
        if (status != MH_OK) {
            logger::error("MH_Initialize failed: {}", static_cast<int>(status));
            return;
        }

        void* target = nullptr;
        status = MH_CreateHookApiEx(
            L"steam_api64.dll",
            "SteamAPI_RunCallbacks",
            reinterpret_cast<void*>(&RunCallbacksHook),
            reinterpret_cast<void**>(&g_originalRunCallbacks),
            &target);
        if (status != MH_OK) {
            logger::error("MH_CreateHookApiEx failed: {}", static_cast<int>(status));
            MH_Uninitialize();
            return;
        }

        status = MH_EnableHook(target);
        if (status != MH_OK) {
            logger::error("MH_EnableHook failed: {}", static_cast<int>(status));
            MH_RemoveHook(target);
            MH_Uninitialize();
            return;
        }

        logger::info("SteamAPI_RunCallbacks hook installed.");
    }

    void OnSKSEMessage(SKSE::MessagingInterface::Message* message)
    {
        if (message && message->type == SKSE::MessagingInterface::kInputLoaded) {
            InstallHook();
        }
    }

    void InitializeLogging()
    {
        auto path = logger::log_directory();
        if (!path) {
            SKSE::stl::report_and_fail("Unable to find the SKSE log directory.");
        }

        *path /= "SteamDeckKeyboardFixSkyrim.log";

        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
        auto log = std::make_shared<spdlog::logger>("global", std::move(sink));
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
    }
}

SKSEPluginInfo(
    .Version = REL::Version{ 0, 3, 2, 0 },
    .Name = "SteamDeckKeyboardFixSkyrim"sv,
    .Author = "Omni"sv,
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = {
        REL::Version{ 1, 6, 1130, 0 },
        REL::Version{ 1, 6, 1170, 0 }
    }
)

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);
    InitializeLogging();

    auto* messaging = SKSE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(OnSKSEMessage)) {
        logger::critical("Failed to register the SKSE messaging listener.");
        return false;
    }

    return true;
}
