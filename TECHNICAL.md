# Technical overview

## Scope

This document describes version 0.3.2 of Steam Deck Keyboard Fix for Skyrim.
It records the observed failure, the containment mechanism and the limits that
keep the intervention narrow. It does not claim to identify or repair the
underlying defect in Skyrim, Steamworks or Proton.

## The observed fault

Bethesda added official Steam Deck support to Skyrim Special Edition in the
1.6.1130 update. Reports of crashes when closing Steam's on-screen keyboard
appeared shortly afterwards, and the problem remains present in 1.6.1170.

Crash logs from affected systems show a null execute access violation while
Steamworks callbacks are running. The instruction pointer is zero, with
`steam_api64.dll` and Proton's `lsteamclient.dll` represented on the stack.
This is sufficient to identify the failure that the plugin contains, but not
to attribute the underlying defect to a particular component.

## What the plugin does

The plugin waits for SKSE's input-loaded message, confirms that the process is
running under Wine or Proton, then finds the copy of `steam_api64.dll` already
loaded by Skyrim. It uses MinHook to place a guard around
`SteamAPI_RunCallbacks` without linking the plugin against Steamworks or
loading another copy of the Steam API library.

Normal Steam callback processing remains unchanged. The original function is
called on every invocation unless it raises the precise exception observed on
affected hardware.

The exception handler accepts only an exception with all of these properties:

- The exception is `EXCEPTION_ACCESS_VIOLATION`.
- The failed operation is an attempt to execute code.
- The attempted target address is zero.
- The exception address is zero.

Any exception that does not match every condition continues through the normal
Windows exception-handling process. It is not hidden by the plugin.

When the known failure occurs, control returns from the guarded callback. The
plugin then asks Steamworks to release memory associated with the current
thread before returning control to Skyrim. A warning is written to the log
once per game session, even if the fault occurs repeatedly.

## Control flow

```text
SKSE loads the plugin
        |
        v
The plugin initializes its log and registers for SKSE messages
        |
        v
SKSE reports that input has loaded
        |
        v
The plugin confirms that Skyrim is running under Wine or Proton
        |
        v
SteamAPI_RunCallbacks is hooked
        |
        v
Steam callbacks run normally inside the narrow exception guard
        |
        +---- no matching fault ----> return normally
        |
        `---- matching fault -------> contain the exception,
                                      release Steam thread memory,
                                      return control to Skyrim
```

## Platform and runtime compatibility

The hook is not installed on native Windows. This avoids changing Steam
callback behaviour outside the environment in which the fault was observed.

The plugin does not use Skyrim addresses, relocation identifiers or
runtime-dependent game structures. One DLL therefore declares compatibility
with both supported runtimes:

- Skyrim 1.6.1130 with SKSE 2.2.5
- Skyrim 1.6.1170 with SKSE 2.2.6

Version 1.6.1170 is the primary test environment. Support for 1.6.1130 is less
tested and compatibility reports are welcome.

The plugin contains no Papyrus scripts, configuration, persistent state or
save-game data. It does not patch Skyrim executable code or use addresses that
vary between the supported runtimes.

## Deliberate limits

The plugin does not replace Steam's keyboard integration, check whether the
keyboard is open, or skip normal callback processing. It does not handle
unrelated access violations. Expanding the accepted exception signature would
require new crash evidence matching the additional case.

The guard is containment for one observed failure signature, not a general
exception handler. A future Skyrim, Steamworks, Proton or SKSE update may
change the failure or make the plugin unnecessary; compatibility must be
re-evaluated if that happens.

Copyright (C) 2026 Omni-Guides. This document and the accompanying source are
licensed under GPL-3.0-only.
