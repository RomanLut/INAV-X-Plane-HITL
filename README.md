# INAV X-Plane HITL plugin

Hardware-in-the-loop plugin for X-Plane for INAV flight controller firmware.

# Motivation

I believe that good testing and debugging tools are key points to achieve software stability.

It is not Ok when people debug autopilot by running with a plane on the field :)

I hope this plugin can help improve INAV firmware.

# How it works

 X-Plane is airplane simulation game. X-Plane is extendable with plugins. This plugin connects for Flight Controller and passes gyroscope, accelerometer, baromener and GPS data, which replace data prom physical sensors. FC sends back yaw/pitch/roll controls which are passed to X-Plane.

# Setup (Windows)

Plugin is Aircraft plugin.

The contents of **release\Aircraft** should be placed in the Aircraft directory of X-Plane:  **X-Plane11\Aircraft\**.

This will add plugin to the **Aerolite** airplane and install additional **NK_Surfwing** flying wing models with plugin.

You have to build and flash Simulator-enabled INAV firmware from branch: https://github.com/RomanLut/inav/tree/master-simulator-xplane 

![](doc/menu.png)

## Installation steps

- Install X-Plane 11 demo version https://www.x-plane.com/desktop/try-it/
- Copy **release\Aircraft** directory to **X-Plane 11\Aircraft** installation directory. 
- Configure FC ( see **sensors** below ).
- Connect FC to PC using USB Cable
- Start X-Plane 11
- Select Aircraft **Aerolite 103**
- Start flight
- Select **Plugins->INAV HITL->Link->Connect to flight controller**. Plugin should automatically detect COM port.
- Flight using RC Controller, setup PIDs etc. Hint: disable brakes with "B" key. 

# Setup (Linux)

 Sorry Linux in not supported currently ( assistance needed to build plugin for the linux).
 
# Sensors emulation

Plugin will emulate sensors, enabled in FC's configuration.

There are two modes of emulation:
- full emulation: attitude is estimated from sensors data
- simplified emulation: attitude is passed from X-Plane.

Due to slow update rate (can not be larger then X-Plan FPS) and no syncronization on beetween INAV cycles and X-Plane FPS, full emulation mode will show noticable problems with estimation ( hickups, drifting horizon ). Simplified emulation is recommended for debugging code not related to IMU.

In minimal case, you need FC with some kind of receiver attached. No barometer and GPS senssors required. OSD is higly recommended.

## Accelerometer and gyroscope

Physical acceleromerer should be calibrated if INAV configurator.

## Barometer

Barometer will be emulated if enabled in INAV configurator. If physical baromenter is not present, select "Fake" type.

## GPS

GPS functionality will be emulated if enabled in INAV configurator. Is is not required to have phisical GPS sensor connected.

## Magnetometer

Magnetometer is not supported currently ( assistance needed to implement magnetometer supports, see [development.md](doc/development.md) ).

Magnetometer will be disabled in simulation.

# OSD 

Plugin will show OSD as configured in INAV configurator. 

Thw following requirements should be met to have OSD in emulator:

- OSD should be configured and enabled in INAV configurator ( AUTO, PAL or NTSC type )
- physical OSD chip should be present on FC (MAX7456). OSD will not work without physical chip.
- X-Plane is *NOT* using Vulkan drivers. Please uncheck the following option:

# Development

See [development.md](doc/development.md)
