# Steam Deck Keyboard Fix for Skyrim

SKSE plugin that prevents Skyrim Special Edition from crashing when Steam's
on-screen keyboard is closed on Steam Deck.

The issue is associated with the Steam Deck support introduced by Bethesda in
Skyrim 1.6.1130 and remains present in 1.6.1170.

Version 0.3.2 is an initial public testing release. It has passed controlled
testing on Steam Deck with Skyrim 1.6.1170 and SKSE 2.2.6. Compatibility with
Skyrim 1.6.1130 is supported but has received less testing so far.

## Requirements

| Skyrim | SKSE | Test status |
|---|---|---|
| 1.6.1130 | 2.2.5 | Supported, less tested so far |
| 1.6.1170 | 2.2.6 | Primary target version |

The plugin activates only when Skyrim is running through Wine or Proton. It
remains inactive on native Windows installations.

## Install

1. Confirm that your Skyrim and SKSE versions match a supported combination
   in the table above.
2. Remove or disable any earlier version of this fix. In particular, make sure
   `SteamDeckKeyboardCrashFix.dll` is not still present in another mod.
3. Download the packaged release ZIP from the GitHub Releases page. Do not use
   GitHub's automatically generated "Source code" archives as the mod package.
4. Install the release ZIP with your mod manager and enable it.
5. Launch Skyrim through SKSE.

For a manual installation, open the release ZIP and copy its `SKSE` folder
into Skyrim's `Data` directory, merging folders when prompted. The installed
DLL should be at:

```text
Data/SKSE/Plugins/SteamDeckKeyboardFixSkyrim.dll
```

The mod contains no ESP, ESL or ESM file, so it does not appear in the plugin
load order. Do not extract the DLL directly into `Data`; retain the
`SKSE/Plugins` directories from the archive.

## Use

No configuration is required. Launch Skyrim through SKSE as usual and you
should now be able to use Steam's on-screen keyboard normally, without
crashes. The fix is active automatically for the duration of the game session.

To remove the mod, disable or uninstall it in your mod manager. For a manual
installation, remove `Data/SKSE/Plugins/SteamDeckKeyboardFixSkyrim.dll`.

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

The warning is written only once per game session. Its absence does not
necessarily indicate a problem; it appears only when the affected callback
actually encounters the known fault. On native Windows, the log instead
reports that the hook was not installed.

## Testing notice

This is an early testing release intended for the supported versions listed
above. The fix is deliberately narrow, has no save-game data, and remains
inactive on native Windows. Even so, keep normal backups of important saves
and report any unexpected behaviour. Installing and using the plugin is at
your own discretion; no software can be guaranteed compatible with every mod
list or future game update.

## Build

Requires MSVC, CMake, Ninja and vcpkg. Set `VCPKG_ROOT`, then run:

```powershell
git submodule update --init --recursive
cmake --preset release
cmake --build build/release
```

## Reporting problems

Bug reports and compatibility results are welcome through GitHub Issues.
Include the following where possible:

- Steam Deck model and SteamOS update channel
- Skyrim and SKSE versions
- Proton version
- Whether the problem occurred at the main menu or in-game
- The steps used to open and close the keyboard
- `SteamDeckKeyboardFixSkyrim.log`
- `skse64.log` and a crash log, if one was produced

Do not include account details or unrelated personal paths in uploaded logs.
See [CONTRIBUTING.md](CONTRIBUTING.md) before submitting code changes.

## Licence

Steam Deck Keyboard Fix for Skyrim is licensed under GPL-3.0-only. Third-party
components retain their respective licences; see
[THIRD_PARTY_LICENSES.txt](THIRD_PARTY_LICENSES.txt).

Copyright (C) 2026 Omni-Guides.
