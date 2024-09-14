#!/bin/bash
set -e

mkdir build
cd build
cmake -GNinja -DOUTPUT_DIR="../release/Aircraft/Extra Aircraft/NK_FPVSurfwing/plugins/INAV-X-Plane-HITL/64" ..
ninja