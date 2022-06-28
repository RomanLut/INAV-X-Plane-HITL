# INAV X-Plane HITL plugin

Hardware-in-the-loop plugin for X-Plane 11 for INAV flight controller firmware: https://github.com/iNavFlight/inav

![](https://camo.githubusercontent.com/5af6d6c7784c1a41290dfa06755dae56ca95a62800495121766f40e597b3cc43/687474703a2f2f7374617469632e726367726f7570732e6e65742f666f72756d732f6174746163686d656e74732f362f312f302f332f372f362f61393038383835382d3130322d696e61762e706e67)

# Motivation

I believe that good testing and debugging tools are key points to achieve software stability.

It is not Ok when people debug autopilot by running with a plane on the field :smiley:

I hope this plugin can help improve INAV firmware.

While not a been a main purpose, plugin can be used to improve pilot skils or getting familiar with INAV options.

# How it works

![](x-plane-logo.png)
 **X-Plane** 11 https://www.x-plane.com/ is flight simulaor with accurate physics simulation. X-Plane is extendable with plugins. This plugin connects for Flight Controller throught USB cable and passes gyroscope, accelerometer, barometer and GPS data, which replace data from physical sensors. FC sends back yaw/pitch/roll controls which are passed to X-Plane.

# Setup (Windows)

Plugin is Aircraft plugin.

The contents of `release\Aircraft` folder should be placed in the Aircraft folder of X-Plane: `X-Plane11\Aircraft\`.

This will add plugin to the **Aerolite** airplane and install additional **NK_Surfwing** flying wing models with plugin.

You have to build and flash Simulator-enabled INAV firmware from branch: https://github.com/RomanLut/inav/tree/master-simulator-xplane 

![](doc/menu.png)

## Installation steps

- Install X-Plane 11 demo version https://www.x-plane.com/desktop/try-it/
- Copy `release\Aircraft` folder to `X-Plane 11\Aircraft` forder in X-plane installation location. 
- Connect FC to PC using USB Cable
- Configure FC (see **Flight Controller configuration** below)
- Start X-Plane 11
- Select Aircraft **Aerolite 103** or **NK Surfwing**
- Start flight
- Select **Plugins->INAV HITL->Link->Connect to flight controller**. Plugin should automatically detect COM port.
- Flight using RC Controller, setup PIDs etc. Hint: disable brakes with "B" key. 

Use **"Internal View/Forward with No Display"** for full immersion FPV flights:
![](doc/internalview.png)

# Setup (Linux)

 Sorry Linux in not supported currently *(assistance needed to build plugin for Linux see [development.md](doc/development.md))*.
 
# Flight controller configuration

Flight controller should be fully configured as for real flight. There is no need to calibrate mixer and motors output.

Configure **Aircraft with tail** for **Aerolite** and **Fixed wing** for **NK Sufwing**. It possible to use plugin with any other models (copy plugin to corresponding Aircraft subdirectory).

Platforms other then "Airplane" are not supported.

*Note: If FC is mouted on model, you can use profiles to have different PIDs for simulated aircraft. Motor and Servo outputs are disabled in simulation mode.*

*Note: There is no need to connect battery, if receiver is powered from USB connection.*

## Pids and rates

See recommended starting pids for **Aerolite** (Aircraft with tail ) [doc/aerolite.md](doc/aerolite.md)

See recommended starting pids for **NK SurfWing** (flying wing) [doc/nksurfwing.md](doc/nksurfwing.md)

# Sensors emulation

Plugin will emulate sensors, enabled in FC's configuration.

In minimal case, you need FC with some kind of receiver attached. No barometer and GPS senssors required. OSD is highly recommended.

![](doc/attitude.png)

There are two modes of emulation:
- full emulation: attitude is estimated from sensors data
- simplified emulation: attitude is passed from X-Plane.

Due to slow update rate (update rate can not be larger then X-Plane FPS) and no synchronization on beetween INAV cycles and X-Plane FPS, full emulation mode will show noticable problems with estimation ( hickups, drifting horizon ). **Simplified emulation is recommended for debugging code not related to IMU**.

## Accelerometer and gyroscope

Physical acceleromerer should be calibrated in INAV configurator.

## Barometer

Barometer will be emulated if enabled in INAV configurator. If physical barometer is not present, select **"Fake"** type.

![](doc/fakebaro.png)

## GPS

GPS functionality will be emulated if enabled in INAV configurator. Is is not required to have phisical GPS sensor connected.

X-Plane simulated work is build on real world data. GPS coordinates in game correspond to real locations. 

Emulatet GPS Fix can be switched in menu:

![](doc/gpsfix.png)

## Magnetometer

Magnetometer is not supported currently *(assistance needed to implement magnetometer support, see [development.md](doc/development.md))*.

Magnetometer will be disabled in simulation.

## Battery sensor

For convience, it is possible to emulate 3S battery presense:

![](doc/battery.png)

# OSD 

Plugin will show OSD exactly as configured in INAV Configurator. 

It is highly recommended to use Flight Controller with OSD to see system messages on screen.

The following requirements should be met to have OSD in emulator:

- OSD should be configured and enabled in INAV configurator ( AUTO, PAL or NTSC type )
- physical OSD chip should be present on FC (MAX7456). OSD will not work without physical MAX7456 compatible chip. OSD will not work with HD Zero, DJI or Pixel OSD.
- X-Plane is *NOT* using Vulkan drivers. Please uncheck the following option:

![](doc/vulkan.png)

The following options present in menu:

![](doc/osdoptions.png)

- **None:** disable OSD rendering
- **AUTO:** number of lines provided by FC
- **PAL:** force render 16 lines
- **NTSC:** force render 13 lines
- **Smoothing: Nearest:** Use linear smoothing for rendering
- **Smoothing: Linear:** Use nearest pixels for rendering

OSD is using **Bold** font from INAV configuraton. It is possibleto replace font `(\plugins\INAV-X-Plane-HITL\64\assets\osd_font.png)` with other font from INAV Configurator https://github.com/iNavFlight/inav-configurator/tree/master/resources/osd

# Beeper

For coonvience, it is possible to mute beeper in simulation mode:

![](doc/beeper.png)


# Development

See [development.md](doc/development.md)

# Links

- X-Plane INAV HITL prototype has been implemented by Sergey Sevrugin: https://github.com/sevrugin/inav/tree/master-simulator-xplane, https://github.com/sevrugin/inav-configurator/tree/master-simulator-xplane

- NK_FPV Surfwing V2 | RC Plane 2.2.0 https://forums.x-plane.org/index.php?/files/file/43974-nk_fpv-surfwing-v2-rc-plane/

- X PLANE TUTORIAL: MaxiSwift installation with X Plane v9.70 for HIL simulations https://github.com/jlnaudin/x-drone/wiki/X-PLANE-TUTORIAL:-MaxiSwift-installation-with-X-Plane-v9.70-for-HIL-simulations

- Quadrotor UAV simulation modelling using X-Plane simulation software http://www.iraj.in/journal/journal_file/journal_pdf/2-448-152361879882-85.pdf
