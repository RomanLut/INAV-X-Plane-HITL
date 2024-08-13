# INAV X-Plane HITL plugin

**Hardware-in-the-loop** plugin for **X-Plane 11 & 12** for **INAV Flight Controller firmware**: 

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/krTDi1tXGX8/0.jpg)](https://www.youtube.com/watch?v=krTDi1tXGX8)

**Hardware-in-the-loop (HITL) simulation**, is a technique that is used in the development and testing of complex real-time embedded systems. 

**X-Plane** is a flight simulation engine series developed and published by Laminar Research https://www.x-plane.com/

**INAV-X-Plane-HITL** is plugin for **X-Plane** for testing and developing **INAV flight controller firmware** https://github.com/iNavFlight/inav.


# Motivation

I believe that good testing and debugging tools are key points to achieve software stability.

It is not Ok when people debug autopilot by running with RC Plane on the field :smiley:

I hope this plugin can help to improve INAV firmware.

While not been a main purpose, plugin can be used to improve pilot skils or getting familiar with INAV settings.

# How it works

![](doc/x-plane-logo.png) 

**X-Plane** https://www.x-plane.com/ is flight simulator with accurate physics simulation. 
 
X-Plane is extendable with plugins. This plugin connects to Flight Controller through USB cable and passes gyroscope, accelerometer, barometer, magnethometer, GPS and Pitot data from X-Plane to FC. Simulated sensors data replaces readings from physical sensors. 

FC sends back **yaw/pitch/roll/trottle** controls which are passed to X-Plane.

## X-Plane 11 or X-Plane 12?

Simulation requires at least 50, or even better 100, solid FPS without freezing. 

While **X-Plane 12** has better visual appearance, **X-Plane 11** is still recommented choice due to better performance. 

Also, small aircraft physics seems to work better in X-Plane 11.

# Setup and usage

See [setup.md](doc/setup.md)

# Development

See [development.md](doc/development.md)

# Special thanks

Many thanks to:
- Sergii Sevriugin for initial implementation and a lot of testing
- NKDesign for NK FPV SurfWing RC plane for X-Plane
- b14ckyy for Surfwing 3D model
- Scavanger for HD OSD and SITL integration
- Bart Slinger for MacOs plugin compilation


# Links

- X-Plane INAV HITL prototype has been orignally implemented by Sergii Sevriugin: 

   https://github.com/sevrugin/inav/tree/master-simulator-xplane

   https://github.com/sevrugin/inav-configurator/tree/master-simulator-xplane

   [![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/XeYr-l9Sowc/0.jpg)](https://www.youtube.com/watch?v=XeYr-l9Sowc)

- NK_FPV Surfwing V2 | RC Plane 2.2.0 

   https://forums.x-plane.org/index.php?/files/file/43974-nk_fpv-surfwing-v2-rc-plane/

- X PLANE TUTORIAL: MaxiSwift installation with X Plane v9.70 for HIL simulations 

   https://github.com/jlnaudin/x-drone/wiki/X-PLANE-TUTORIAL:-MaxiSwift-installation-with-X-Plane-v9.70-for-HIL-simulations

- Quadrotor UAV simulation modelling using X-Plane simulation software 

   http://www.iraj.in/journal/journal_file/journal_pdf/2-448-152361879882-85.pdf
