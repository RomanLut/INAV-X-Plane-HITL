cls
rem rd /s /q "./src/build"
docker run --rm -it -u root -v //d/Github/inav/INAV-X-Plane-HITL-Plugin/INAV-X-Plane-HITL/:/trunk inav-hitl-build 
copy /y src\build\INAV-X-Plane-HITL\64\lin.xpl "release\Aircraft\Extra Aircraft\NK_FPVSurfwing\plugins\INAV-X-Plane-HITL\64\"
copy /y src\build\INAV-X-Plane-HITL\64\lin.xpl "release\Aircraft\Laminar Research\Aerolite 103\plugins\INAV-X-Plane-HITL\64\"


