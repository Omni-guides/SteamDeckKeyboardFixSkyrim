# Technical overview

## The fault

Bethesda added official Steam Deck support to Skyrim Special Edition in the
1.6.1130 update. Reports of crashes when closing Steam's on-screen keyboard
appeared shortly afterwards, and the problem remains present in 1.6.1170.

Crash logs from affected systems show a null execute access violation while
Steamworks callbacks are running. The instruction pointer is zero, with
`steam_api64.dll` and Proton's `lsteamclient.dll` represented on the stack.
This identifies the failure that the plugin contains, but does not establish
the deeper cause inside Steamworks, Proton or Skyrim.

## What the plugin does

The plugin waits for SKSE's input-loaded message, then finds the copy of
`steam_api64.dll` already loaded by Skyrim. Under Wine or Proton it uses
MinHook to redirect `SteamAPI_RunCallbacks` through a small guard function.

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

When the known failure occurs, control returns from the guarded callback and
the plugin calls `SteamAPI_ReleaseCurrentThreadMemory`. A warning is written to
the log once per game session, even if the fault occurs repeatedly.

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
callback behaviour where the Proton-specific failure cannot occur.

The plugin does not use Skyrim addresses, relocation identifiers or
runtime-dependent game structures. One DLL therefore declares compatibility
with both supported runtimes:

- Skyrim 1.6.1130 with SKSE 2.2.5
- Skyrim 1.6.1170 with SKSE 2.2.6

Version 1.6.1170 is the primary test environment. Support for 1.6.1130 is less
tested and compatibility reports are welcome.

## Deliberate limits

The plugin does not replace Steam's keyboard integration, check whether the
keyboard is open, or skip normal callback processing. It does not handle
unrelated access violations. Expanding the accepted exception signature would
require new crash evidence matching the additional case.
