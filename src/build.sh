#!/bin/bash
set -e

make "INAV-X-Plane-HITL"

ldd build/INAV-X-Plane-HITL/64/lin.xpl

objdump -x build/INAV-X-Plane-HITL/64/lin.xpl