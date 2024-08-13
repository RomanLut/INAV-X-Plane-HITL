#!/bin/bash
set -e

make lin64

ldd "../release/Aircraft/Extra Aircraft/NK_FPVSurfwing/plugins/INAV-X-Plane-HITL/64/lin.xpl"

objdump -x "../release/Aircraft/Extra Aircraft/NK_FPVSurfwing/plugins/INAV-X-Plane-HITL/64/lin.xpl"