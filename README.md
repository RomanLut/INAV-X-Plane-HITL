# INAV X-Plane HITL plugin

**Hardware-in-the-loop** plugin for **X-Plane 11** for **INAV Flight Controller firmware**: 

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/krTDi1tXGX8/0.jpg)](https://www.youtube.com/watch?v=krTDi1tXGX8)

**Hardware-in-the-loop (HITL) simulation**, is a technique that is used in the development and testing of complex real-time embedded systems. 

**X-Plane** is a flight simulation engine series developed and published by Laminar Research https://www.x-plane.com/

**INAV-X-Plane-HITL** is plugin for **X-Plane** for testing and developing flight controllers with **INAV flight controller firmware** https://github.com/iNavFlight/inav.


# Motivation

I believe that good testing and debugging tools are key points to achieve software stability.

It is not Ok when people debug autopilot by running with RC Plane on the field :smiley:

I hope this plugin can help improve INAV firmware.

While not been a main purpose, plugin can be used to improve pilot skils or getting familiar with INAV settings.

# How it works

![](doc/x-plane-logo.png) 

**X-Plane** 11 https://www.x-plane.com/ is flight simulator with accurate physics simulation. 
 
X-Plane is extendable with plugins. This plugin connects to Flight Controller through USB cable and passes gyroscope, accelerometer, barometer, magnethometer and GPS data, which replace data from physical sensors. 

FC sends back **yaw/pitch/roll** controls which are passed to X-Plane.

# Setup (Windows)

Plugin is **Aircraft** plugin.

The contents of `release\Aircraft` folder should be placed in the Aircraft folder of X-Plane: `X-Plane11\Aircraft\`.

This will add plugin to the **Aerolite** airplane and install additional **NK_Surfwing** flying wing model with this plugin.

**IMPORTANT:: You have to flash Simulator-enabled INAV firmware. Take precompiled binaries from release https://github.com/RomanLut/INAV-X-Plane-HITL/releases/, or build yourself from branch: https://github.com/RomanLut/inav/tree/simulator_v0.5 . Simulator mode is not included in main INAV repository currently.**

![](doc/menu.png)

## Installation steps

- Install **X-Plane 11 demo** from: https://www.x-plane.com/desktop/try-it/
  Demo allows to fly for 10 minutes in Seattle region. Than you have to restart the game.
- download **Aircraft.zip** from last release https://github.com/RomanLut/INAV-X-Plane-HITL/releases/
- Unzip, copy `Aircraft` folder to `X-Plane 11\Aircraft` folder in **X-Plane** installation location. 
- Connect Flight Controller to PC using USB Cable. **Make sure FC is flashed with simulator-enabled firmware**  https://github.com/RomanLut/INAV-X-Plane-HITL/releases/
- Configure FC (see **Flight Controller configuration** below)
- Start **X-Plane 11**
- Select "Show extra aircraft from older versions" to see NK_FPVSurfwing
![](doc/show_extra_aircraft.jpg)
- Select Airplane **NK Surfwing** or **Aerolite 103**
- Start flight
- Select **Plugins->INAV HITL->Link->Connect to flight controller**. Plugin should automatically detect COM port.
- Flight using RC Controller, setup PIDs etc. **Hint: disable parking brakes with "B" key on takeoff if necessary.** 


*Note:Use **"Internal View/Forward with No Display"** for full immersion FPV flights:*

![](doc/internalview.png)

# Setup (Linux)

 For Linux, all steps are analogous to Windows.

# Setup (MacOs)

 Sorry, MacOs in not supported currently *(assistance needed to build plugin for MacOs, see [development.md](doc/development.md))*.

# Flight controller configuration

Flight controller should be fully configured like for the real flight. There is no need to calibrate mixer and motors output.

Configure **Aircraft with tail** for **Aerolite** and **Flying wing** for **NK Sufwing**. It is possible to use plugin with any other airplane (copy plugin to corresponding Aircraft subfolder).

Platforms other than "Airplane" are not supported.

*Note: If FC is mouted on UAV, you can use profiles to have different PIDs for simulated airplane. Motor and Servo outputs are disabled in simulation mode.*

*Note: There is no need to connect battery, if receiver is powered from USB connection.*

## Pids and rates

See recommended starting pids for **Aerolite** (Aircraft with tail ) [aerolite.md](doc/aerolite/aerolite.md)

See recommended starting pids for **NK SurfWing** (flying wing) [nkfpvsurfwing.md](doc/nkfpvsufrwing/nkfpvsurfwing.md)

# Sensors simulation

Plugin will simulate sensors, enabled in FC's configuration.

In minimal case, you need FC with some kind of receiver attached. No real barometer and GPS sensors are required. FC with OSD is highly recommended.

![](doc/attitude.png)

There are two modes of simulation:
- **simplified simulation:** attitude is passed from X-Plane.
- **full simulation:** attitude is estimated from sensors data

Due to slow update rate (update rate can not be larger then X-Plane FPS) and no synchronization beetween INAV cycles and **X-Plane** FPS, full emulation mode will show noticeable problems with estimation ( drifting horizon ). **Simplified emulation is recommended for debugging code not related to IMU**.

Sensors not listed below are not simulated. Please disable in INAV configuration.

## Accelerometer and gyroscope

Real acceleromerer should be calibrated in **INAV Configurator**. Accelerometer orientation settings are not relevant for simulation.

## Barometer

Barometer will be simulated if enabled in **INAV Configurator**. If real barometer is not present, select **"FAKE"** type.

![](doc/fakebaro.png)

## GPS

GPS functionality will be emulated if enabled in **INAV Configurator**. It is not required to have real GPS sensor connected.

X-Plane simulated world is build based on real world map data. GPS coordinates in game correspond to real locations. 

Simulated GPS Fix can be switched in menu:

![](doc/gpsfix.png)

## Magnetometer

Magnetometer is simulated if selected in INAV configurator. It is not required to connect real sensor. Select QMC5883 if there is no real sensor present. Calibration is not required.

## Battery sensor

For convience, it is possible to simulate 3S battery presense:

![](doc/battery.png)

# OSD 

OSD is rendered in **X-Plane** exactly as configured in **INAV Configurator**. 

![](doc/osd.jpg)

It is highly recommended to use Flight Controller with OSD to see system messages on screen.

The following requirements should be met to have OSD drawn in **X-Plane**:

- OSD should be configured and enabled in **INAV Configurator** ( `AUTO`, `PAL` or `NTSC` type ).
- OSD chip should be present on Flight Controller (MAX7456). OSD will not work without MAX7456 compatible chip. OSD will not work with HD Zero, DJI or Pixel OSD selected in configuration.
- **X-Plane** should **NOT** use **Vulkan** drivers. Please uncheck the following option:

![](doc/vulkan.png)

The following options present in menu:

![](doc/osdoptions.png) 

- **None:** disable OSD rendering
- **AUTO:** number of OSD lines is provided by FC
- **PAL:** force rendering of 16 OSD lines
- **NTSC:** force rendering of 13 OSD lines
- **Smoothing: Nearest:** Use nearest pixels for charactes rendering
- **Smoothing: Linear:** Use linear smoothing for charactes rendering

OSD is using **Bold** font from **INAV Configurator**. It is possible to replace font `(\plugins\INAV-X-Plane-HITL\64\assets\osd_font.png)` with other font from **INAV Configurator** https://github.com/iNavFlight/inav-configurator/tree/master/resources/osd

# Beeper

For coonvience, it is possible to mute beeper in simulation mode:

![](doc/beeper.png)

# Analog Video link quality simulation

Plugin will simulate analog video link quality acccording to setting. Home point is independent from FC home point. Home point is set to position on ARM.

![](doc/noise.jpg)

# Flight path drawing

Plugin can draw flight path on X-Plane map. 

![](doc/menu_map.jpg)

Coordinates can be extracted from:
- **Latitude/Longitude from OSD** -  extracted from OSD. Note that these number have low accuracy and are not updated synchronously. Path will have jittering under zoom.
- **debug[0]/debug[1] as Latitude/Longitude** - assumed that coordinates are provided in debug[0]/debug[1] variables by custom INAV build (int32_t, int32_t).

Path is drawn on "INAV HITL" layer:
![](doc/map.png)


# Servo autotrim

Autotrim and servo autotrim have no effect in simulation (TODO).

# Development

See [development.md](doc/development.md)

# Links

- X-Plane INAV HITL prototype has been implemented by Sergii Sevriugin: 

   https://github.com/sevrugin/inav/tree/master-simulator-xplane

   https://github.com/sevrugin/inav-configurator/tree/master-simulator-xplane

   [![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/XeYr-l9Sowc/0.jpg)](https://www.youtube.com/watch?v=XeYr-l9Sowc)

- NK_FPV Surfwing V2 | RC Plane 2.2.0 

   https://forums.x-plane.org/index.php?/files/file/43974-nk_fpv-surfwing-v2-rc-plane/

- X PLANE TUTORIAL: MaxiSwift installation with X Plane v9.70 for HIL simulations 

   https://github.com/jlnaudin/x-drone/wiki/X-PLANE-TUTORIAL:-MaxiSwift-installation-with-X-Plane-v9.70-for-HIL-simulations

- Quadrotor UAV simulation modelling using X-Plane simulation software 

   http://www.iraj.in/journal/journal_file/journal_pdf/2-448-152361879882-85.pdf
