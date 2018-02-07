#******************************
# Created by Juan-Pablo Caceres
#******************************

CONFIG += c++11 console
CONFIG -= app_bundle

CONFIG += qt thread debug_and_release build_all
CONFIG(debug, debug|release) {
  TARGET = jacktrip_debug
  } else {
  TARGET = jacktrip
  }

QT -= gui
QT += network
# http://wiki.qtcentre.org/index.php?title=Undocumented_qmake#Custom_tools
#cc DEFINES += __RT_AUDIO__
# Configuration without Jack
nojack {
  DEFINES += __NO_JACK__
}
!win32 {
  INCLUDEPATH+=/usr/local/include
  LIBS += -L/usr/local/lib -ljack -lm
  nojack {
    message(Building NONJACK)
    LIBS -= -ljack
  }
}

macx {
  message(MAC OS X)
  QMAKE_CXXFLAGS += -D__MACOSX_CORE__ #-D__UNIX_JACK__ #RtAudio Flags
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
  #QMAKE_MAC_SDK = macosx10.9
  CONFIG -= app_bundle
  #CONFIG += x86 #ppc #### If you have both libraries installed, you
  # can change between 32bits (x86) or 64bits(x86_64) Change this to go back to 32 bits (x86)
  LIBS += -framework CoreAudio -framework CoreFoundation
  DEFINES += __MAC_OSX__
  }

linux-g++ | linux-g++-64 {
#   LIBS += -lasound -lrtaudio
  QMAKE_CXXFLAGS += -D__LINUX_ALSA__ #-D__LINUX_OSS__ #RtAudio Flags
  QMAKE_CXXFLAGS += -g -O2
  DEFINES += __LINUX__
  }

linux-g++ {
  message(Linux)
  QMAKE_CXXFLAGS += -D__LINUX_ALSA__ #-D__LINUX_OSS__ #RtAudio Flags
  }

linux-g++-64 {
  message(Linux 64bit)
  QMAKE_CXXFLAGS += -fPIC -D__LINUX_ALSA__ #-D__LINUX_OSS__ #RtAudio Flags
  }


win32 {
  message(win32)
#cc  CONFIG += x86 console
CONFIG += c++11 console
INCLUDEPATH += C:\Users\cc\Downloads\jack2-master\jack2-master\common
LIBS += -LC:\Users\cc\Documents\jackSimpleClient\lib -ljack
#cc  QMAKE_CXXFLAGS += -D__WINDOWS_ASIO__ #-D__UNIX_JACK__ #RtAudio Flags
  #QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++ -lpthread
  LIBS += -lWs2_32 #cc -lOle32 #needed by rtaudio/asio
#cc  LIBS += "../externals/includes/QTWindows/libjack.lib"
  DEFINES += __WIN_32__
#cc    DEFINES -= UNICODE #RtAudio for Qt
INCLUDEPATH += ../../Downloads/jack2-master/jack2-master/common
LIBS += -LC:\Users\cc\Documents\jackSimpleClient\lib -ljack
}




DESTDIR = .
QMAKE_CLEAN += -r ./jacktrip ./jacktrip_debug ./release ./debug
target.path = /usr/bin
INSTALLS += target


# Input
HEADERS += DataProtocol.h \
           JackTrip.h \
           jacktrip_globals.h \
           jacktrip_types.h \
           JackTripThread.h \
           JackTripWorker.h \
           JackTripWorkerMessages.h \
           LoopBack.h \
           NetKS.h \
           PacketHeader.h \
           ProcessPlugin.h \
           RingBuffer.h \
           RingBufferWavetable.h \
           Settings.h \
           TestRingBuffer.h \
           ThreadPoolTest.h \
           UdpDataProtocol.h \
           UdpMasterListener.h \
           AudioInterface.h
           #JamTest.h
!nojack {
SOURCES += JackAudioInterface.h
}
SOURCES += DataProtocol.cpp \
           JackTrip.cpp \
           jacktrip_globals.cpp \
           jacktrip_main.cpp \
           jacktrip_tests.cpp \
           JackTripThread.cpp \
           JackTripWorker.cpp \
           LoopBack.cpp \
           PacketHeader.cpp \
           ProcessPlugin.cpp \
           RingBuffer.cpp \
           Settings.cpp \
           #tests.cpp \
           UdpDataProtocol.cpp \
           UdpMasterListener.cpp \
           AudioInterface.cpp
!nojack {
SOURCES += JackAudioInterface.cpp
}

# RtAduio Input
win32 {
  INCLUDEPATH += ../externals/rtaudio-4.1.1/include
  DEPENDPATH += ../externals/rtaudio-4.1.1/include
}
macx | win32 {
INCLUDEPATH += ../externals/rtaudio-4.1.1/
DEPENDPATH += ../externals/rtaudio-4.1.1/
HEADERS +=
SOURCES +=
}

