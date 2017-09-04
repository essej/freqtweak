
# Copyright (C) 2002 Jesse Chappell <jesse@essej.net>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


COMMON_FLAGS =  -Wall -D_REENTRANT \
		 @ARCH_CFLAGS@

#AM_CXXFLAGS = $(COMMON_FLAGS) $(EXTRA_CFLAGS) $(JACK_CFLAGS) $(FFTW_CFLAGS) @XML_CFLAGS@

#AM_CFLAGS = $(AM_CXXFLAGS)

EXTRA_DIST = ftlogo.xpm $(wildcard pixmaps/*.xpm)

#freqtweak_LDADD = @JACK_LIBS@ @XML_LIBS@


bin_PROGRAMS = freqtweak

freqtweak_SOURCES = \
	FTapp.cpp \
	FTmainwin.cpp \
	FTioSupport.cpp \
	FTjackSupport.cpp \
	FTprocessPath.cpp \
	FTspectralEngine.cpp \
	FTspectragram.cpp \
	FTspectrumModifier.cpp \
	FTactiveBarGraph.cpp \
	FTutils.cpp \
	FTportSelectionDialog.cpp\
	FTconfigManager.cpp \
	RingBuffer.cpp \
	xml++.cpp \
	xml++.hpp \
	FTapp.hpp \
	FTmainwin.hpp \
	FTjackSupport.hpp \
	FTioSupport.hpp \
	FTprocessPath.hpp \
	FTtypes.hpp \
	FTspectralEngine.hpp \
	FTspectragram.hpp \
	FTspectrumModifier.hpp \
	FTactiveBarGraph.hpp \
	FTutils.hpp \
	FTportSelectionDialog.hpp \
	FTconfigManager.hpp \
	FTupdateToken.hpp \
	RingBuffer.hpp \
	FTprocI.cpp \
	FTprocI.hpp \
	FTprocDelay.cpp \
	FTprocDelay.hpp \
	FTprocEQ.cpp \
	FTprocEQ.hpp \
	FTprocGate.cpp \
	FTprocGate.hpp \
	FTprocPitch.cpp \
	FTprocPitch.hpp \
	FTprocOrderDialog.cpp \
	FTprocOrderDialog.hpp \
	FTdspManager.cpp \
	FTdspManager.hpp \
	FTprocLimit.cpp \
	FTprocLimit.hpp \
	FTprocWarp.cpp \
	FTprocWarp.hpp \
	FTpresetBlendDialog.cpp \
	FTpresetBlendDialog.hpp \
	FTpresetBlender.cpp \
	FTpresetBlender.hpp \
	FTprocCompressor.cpp \
	FTprocCompressor.hpp \
	FTprocBoost.cpp \
	FTprocBoost.hpp \
	FThelpWindow.cpp \
	FThelpWindow.hpp \
	FTmodulatorI.cpp \
	FTmodulatorI.hpp \
	FTmodulatorManager.cpp \
	FTmodulatorManager.hpp \
	FTmodulatorDialog.cpp \
	FTmodulatorDialog.hpp \
	FTmodulatorGui.cpp \
	FTmodulatorGui.hpp \
	FTmodRandomize.cpp \
	FTmodRandomize.hpp \
	FTmodRotate.cpp \
	FTmodRotate.hpp \
	FTmodRotateLFO.cpp \
	FTmodRotateLFO.hpp \
	FTmodValueLFO.cpp \
	FTmodValueLFO.hpp \
	LockMonitor.hpp \
	cycles.h \
	pix_button.hpp \
	pix_button.cpp \
	slider_bar.hpp \
	slider_bar.cpp \
	spin_box.hpp \
	spin_box.cpp \
	pixmap_includes.hpp

mac: freqtweak
	/Developer/Tools/Rez -d __DARWIN__ -t APPL Carbon.r -o freqtweak

