# Development

Plugin is compiled using Microsoft Visual Studio 2017.

## Concerns

Existing MSP commands should not be changed.

Whole simulator communication should be done using single MSP_SIMULATOR command and ARM_SIMULATION flag.

If ARM_SIMULATION flag is not set, behaviour of INAV should not change at all.

For now, plugin supports Aircarft type "Aircraft with tail" only

## Datarefs

Some datarefs are available under **inavhitl/** node for debugging.

