
# HOTSPOT

include common_variables.mk

LIB_energy_arm = ../lib
LIB_hotspot = ./LibHotspot

CXX= g++-4.9

CFLAGS+= -O3 -I . 
CFLAGS+= -Wno-unused-result -Wno-write-strings -Wno-deprecated-register
CFLAGSA = $(CFLAGS) -std=c++11
#XI_CFLAGS = $(CFLAGS) -DXILINX --std=gnu++11 -I $(INCLUDE_TBB_ARM) 
#XI_CFLAGS = $(CFLAGS) -DXILINX --std=gnu++11 -march=armv7-a -I $(INCLUDE_TBB_ARM) 
XI_CFLAGS = $(CFLAGS) -DXILINX --std=gnu++11 -march=armv7-a -I $(INCLUDE_TBB_ARM) -I LibHotspot -mfloat-abi=hard
#XI_CFLAGS = $(CFLAGS) -DXILINX --std=gnu++11 -I $(INCLUDE_TBB_ARM) -I LibkernelHotspot
XI_CFLAGS+= -DNEON -mfpu=neon -funsafe-math-optimizations -ftree-vectorize -mvectorize-with-neon-quad -ftree-vectorizer-verbose=2 #If NEON SIMD instructions are to be used
#XI_CFLAGS+= -mfloat-abi=soft


#CFALGS+= -DTRACE  # to get trace file output. Use ViTE to visualize

#LDFLAGS =  -ltbb -pthread -ltbbmalloc_proxy -ltbbmalloc # -lgopt 


LDFLAGS =  -ltbb -pthread -ltbbmalloc_proxy -ltbbmalloc

#XI_LDFLAGS = $(LDFLAGS) -L $(LIB_TBB_ARM) -L $(LIB_energy_arm) 
#XI_LDFLAGS = $(LDFLAGS) -L $(LIB_TBB_ARM) -L $(LIB_energy_arm) ./LibHotspot/libkernelnbody.a
XI_LDFLAGS = $(LDFLAGS) -L $(LIB_TBB_ARM) -L $(LIB_energy_arm) -L $(LIB_hotspot)
#XI_LDFLAGS = $(LDFLAGS) -L $(LIB_TBB_ARM) -L $(LIB_energy_arm)
XI_LDFLAGS+= -lenergy_meter_arm -lrt -lkernelHotspot# -lPVROCL
#XI_LDFLAGS+= -lenergy_meter_arm -lrt

XI_PROGs= HOTSPOT_DSW 
#NBODYV NBODY_FS

PROFILING= HOTSPOT_PROCL 

SOURCE= hotspot.cpp  ../Schedulers/FixedScheduler.h ../Schedulers/DynamicScheduler.h ../Schedulers/StaticScheduler.h kernelHotspot.h

.PHONY: all clean odroid libraries

all: odroid

libraries:
	$(MAKE) -C ../energy-meter

odroid: libraries $(XI_PROGs)

HOTSPOT_FS: $(SOURCE) 
	$(CXX) -o $@ $< $(XI_CFLAGS) $(XI_LDFLAGS) -DSCHED=FIXS 

HOTSPOT_DS: $(SOURCE) 
	$(CXX) -o $@ $< $(XI_CFLAGS) $(XI_LDFLAGS) -DSCHED=DYNAMIC -DONETIMESTEP=5

HOTSPOT_DSW: $(SOURCE) 
	$(CXX) -o $@ $< $(XI_CFLAGS) $(XI_LDFLAGS) -DSCHED=DYNAMIC -DWRITERESULT 
	#-DHOSTPRI_LINUX

HOTSPOT_SS: $(SOURCE) 
	$(CXX) -o $@ $< $(XI_CFLAGS) $(XI_LDFLAGS) -DSCHED=SS -DONETIMESTEP=5

HOTSPOT_b: $(SOURCE) 
	$(CXX) -o $@ $< $(XI_CFLAGS) $(XI_LDFLAGS) -DSCHED=FIXS -DONETIMESTEP=5

HOTSPOTV: $(SOURCE) 
	$(CXX) -o $@ $< $(XI_CFLAGS) $(XI_LDFLAGS) -DSCHED=FIXS -DVERBOSE=1

HOTSPOT_NOENERGY: $(SOURCE) 
	$(CXX) -o $@ $< $(XI_CFLAGS) $(XI_LDFLAGS) -DSCHED=FIXS -DONETIMESTEP=15 -DNOENERGY

HOTSPOT_PRIO: $(SOURCE) 
	$(CXX) -o $@ $< $(XI_CFLAGS) $(XI_LDFLAGS) -DHOSTPRI_LINUX -DSCHED=FIXS 

oracle: HOTSPOT_SS

#define GPU_THRESHOLD 10 

mem_test: mem_test.cpp
	$(CXX) -o $@ $< $(XI_CFLAGS) $(XI_LDFLAGS) 

clean:
	rm -f  $(XI_PROGs) HOTSPOT_SS 

