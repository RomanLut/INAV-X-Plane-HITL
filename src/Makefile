BUILDDIR	:=	./build
SRC_BASE	:=	.
TARGET		:= INAV-X-Plane-HITL

SOURCES = \
	GL/glew.c \
	stb/stbi_image.cpp \
	main.cpp \
	graph.cpp \
	menu.cpp \
	msp.cpp \
	osd.cpp \
	serialbase.cpp \
	serial.cpp \
	tcpserial.cpp \
	simData.cpp \
	stats.cpp \
	util.cpp \
	sound.cpp \
	map.cpp \
    fontbase.cpp \
    fontanalog.cpp \
    fontwalksnail.cpp \
    fonthdzero.cpp \
    fontwtfos.cpp \
    ipinputwidget.cpp \
	osdRenderer.cpp \
	shader.cpp

LIBS = -lopenal

INCLUDES = \
	-I$(SRC_BASE)/SDK/CHeaders/XPLM \
	-I$(SRC_BASE)/SDK/CHeaders/Widgets \
	-I$(SRC_BASE)/GL


############################################################################


VPATH = $(SRC_BASE)

CSOURCES	:= $(filter %.c, $(SOURCES))
CXXSOURCES	:= $(filter %.cpp, $(SOURCES))

CDEPS64			:= $(patsubst %.c, $(BUILDDIR)/obj64/%.cdep, $(CSOURCES))
CXXDEPS64		:= $(patsubst %.cpp, $(BUILDDIR)/obj64/%.cppdep, $(CXXSOURCES))
COBJECTS64		:= $(patsubst %.c, $(BUILDDIR)/obj64/%.o, $(CSOURCES))
CXXOBJECTS64	:= $(patsubst %.cpp, $(BUILDDIR)/obj64/%.o, $(CXXSOURCES))
ALL_DEPS64		:= $(sort $(CDEPS64) $(CXXDEPS64))
ALL_OBJECTS64	:= $(sort $(COBJECTS64) $(CXXOBJECTS64))



# Phony directive tells make that these are "virtual" targets, even if a file named "clean" exists.
.PHONY: all clean $(TARGET)
# Secondary tells make that the .o files are to be kept - they are secondary derivatives, not just
# temporary build products.
.SECONDARY: $(ALL_OBJECTS) $(ALL_OBJECTS64) $(ALL_DEPS)

DEFINES = -DXPLM200=1 -DXPLM210=1 -DXPLM300=1 -DXPLM301=1 -DXPLM302=1 -DXPLM303=1

# Target rules - these just induce the right .xpl files.
$(TARGET): lin64
lin64: $(BUILDDIR)/../../release/Aircraft/Extra\ Aircraft/NK_FPVSurfwing/plugins/INAV-X-Plane-HITL/64/lin.xpl
$(BUILDDIR)/../../release/Aircraft/Extra\ Aircraft/NK_FPVSurfwing/plugins/INAV-X-Plane-HITL/64/lin.xpl: DEFINES += -DAPL=0 -DIBM=0 -DLIN=1

mac64: $(BUILDDIR)/../../release/Aircraft/Extra\ Aircraft/NK_FPVSurfwing/plugins/INAV-X-Plane-HITL/64/mac.xpl
$(BUILDDIR)/../../release/Aircraft/Extra\ Aircraft/NK_FPVSurfwing/plugins/INAV-X-Plane-HITL/64/mac.xpl: \
	DEFINES += -arch x86_64 -DAPL=1 -DIBM=0 -DLIN=0 -DGL_SILENCE_DEPRECATION 
	LIBDIRS := -F$(SRC_BASE)/SDK/Libraries/Mac
	LIBS2   := -framework XPLM -framework XPWidgets -framework OpenGL -framework GLUT -framework OpenAL -framework CoreFoundation -lc++
	LDFLAGS := $(LIBDIRS) $(LIBS2)


CFLAGS := -fPIC -fvisibility=hidden `pkg-config --cflags --libs gtk+-3.0` --std=c++17
CFLAGS2 := -fPIC -fvisibility=hidden `pkg-config --cflags` --std=c++17

$(BUILDDIR)/../../release/Aircraft/Extra\ Aircraft/NK_FPVSurfwing/plugins/INAV-X-Plane-HITL/64/lin.xpl: $(ALL_OBJECTS64)
	@echo Linking $@
	gcc -m64 -static-libgcc -shared -Wl,--version-script=exports.txt -o "$@" $(ALL_OBJECTS64) $(LIBS)

$(BUILDDIR)/../../release/Aircraft/Extra\ Aircraft/NK_FPVSurfwing/plugins/INAV-X-Plane-HITL/64/mac.xpl: $(ALL_OBJECTS64)
	@echo Linking $@
	gcc -m64 -shared -arch x86_64 -Wl -o "$@" $(ALL_OBJECTS64) $(LDFLAGS) $(CFLAGS2)


# Compiler rules

# What does this do?  It creates a dependency file where the affected
# files are BOTH the .o itself and the cdep we will output.  The result
# goes in the cdep.  Thus:
# - if the .c itself is touched, we remake the .o and the cdep, as expected.
# - If any header file listed in the cdep turd is changed, rebuild the .o.
$(BUILDDIR)/obj64/%.o : %.c
	mkdir -p $(dir $@)
	g++ $(DEFINES) $(INCLUDES) $(CFLAGS) -m64 -c $< -o $@
	g++ $(DEFINES) $(INCLUDES) $(CFLAGS) -MM -MT $@ -o $(@:.o=.cdep) $<

$(BUILDDIR)/obj64/%.o : %.cpp
	mkdir -p $(dir $@)
	g++ $(DEFINES) $(INCLUDES) $(CFLAGS) -m64 -c $< -o $@
	g++ $(DEFINES) $(INCLUDES) $(CFLAGS) -MM -MT $@ -o $(@:.o=.cppdep) $<

clean:
	@echo Cleaning out everything.
	rm -rf $(BUILDDIR)

# Include any dependency turds, but don't error out if they don't exist.
# On the first build, every .c is dirty anyway.  On future builds, if the
# .c changes, it is rebuilt (as is its dep) so who cares if dependencies
# are stale.  If the .c is the same but a header has changed, this
# declares the header to be changed.  If a primary header includes a
# utility header and the primary header is changed, the dependency
# needs a rebuild because EVERY header is included.  And if the secondary
# header is changed, the primary header had it before (and is unchanged)
# so that is in the dependency file too.
-include $(ALL_DEPS64)


