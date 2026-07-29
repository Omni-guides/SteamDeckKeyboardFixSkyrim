# Steam Deck Keyboard Fix for Skyrim

SKSE plugin that prevents Skyrim Special Edition from crashing when Steam's
on-screen keyboard is closed on Steam Deck.

The issue is associated with the Steam Deck support introduced by Bethesda in
Skyrim 1.6.1130 and remains present in 1.6.1170.

## Requirements

| Skyrim | SKSE | Test status |
|---|---|---|
| 1.6.1130 | 2.2.5 | Supported, less tested so far |
| 1.6.1170 | 2.2.6 | Primary target version |

The plugin activates only when Skyrim is running through Wine or Proton. It
remains inactive on native Windows installations.

## Install

Install the release archive with a mod manager and enable the mod. For a
manual installation, copy the DLL to:

```text
Data/SKSE/Plugins/SteamDeckKeyboardFixSkyrim.dll
```

The mod contains no ESP, ESL or ESM file, so it does not appear in the plugin
load order.

## Use

No configuration is required. Launch Skyrim through SKSE as usual and you
should now be able to use Steam's on-screen keyboard normally, without
crashes. The fix is active automatically for the duration of the game session.

## How it works

The plugin hooks `SteamAPI_RunCallbacks`. Normal callback processing is
unchanged. If callback dispatch raises the null execute violation observed on
hardware, the plugin handles that exception and returns control to the game.
Other exceptions are not handled.

See [TECHNICAL.md](TECHNICAL.md) for the observed crash signature, control
flow and safety limits.

## Log

The plugin writes `SteamDeckKeyboardFixSkyrim.log` to the SKSE log directory.
When the known failure is encountered and contained, the log includes:

```text
Contained a null execute violation from SteamAPI_RunCallbacks.
```

## Build

Requires MSVC, CMake, Ninja and vcpkg. Set `VCPKG_ROOT`, then run:

```powershell
git submodule update --init --recursive
cmake --preset release
cmake --build build/release
```

## Reporting problems

Bug reports and compatibility results are welcome through GitHub Issues.
Include the Skyrim version, SKSE version and plugin log where possible. See
[CONTRIBUTING.md](CONTRIBUTING.md) before submitting code changes.

## Licence

Steam Deck Keyboard Fix for Skyrim is licensed under GPL-3.0-only. Third-party
components retain their respective licences; see
[THIRD_PARTY_LICENSES.txt](THIRD_PARTY_LICENSES.txt).
