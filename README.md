# INAV X-Plane HITL plugin

Hardware-in-the-loop plugin for X-Plane for INAV flight controller firmware

# Motivation

I beleave that good testing and debugging tools are key points to achieve software statbility.

# Setup (Windows)

Plugin is Aircraft plugin.

Plugin should be placed in Aircraft directory:  *X-Plane11\Aircraft\Laminar Research\Aerolite 103\plugins\INAV-X-Plane-HITL\*

You have to build and flash Simulator-enabled INAV firmware from: https://github.com/RomanLut/inav/tree/master-simulator-xplane

- Install X-Plane 11 demo version https://www.x-plane.com/desktop/try-it/

- Copy *release\Aircraft* directory to *X-Plane 11\Aircraft* installation directory. 

- Connect FC to PC using USB Cable

- Start X-Plane 11

- Select Aircraft *Aerolite 103*

- Start flight

- Select *Plugins->INAV HITL->Link->Connect to flight controller*


# Development

See [development.md](docs/development.md)