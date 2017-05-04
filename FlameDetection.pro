QT += core
QT += core gui
QT += network

CONFIG += c++11

TARGET = FlameDetection
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    utils.cpp \
    VideoHandler.cpp \
    FeatureAnalyzer.cpp \
    FlameDecider.cpp \
    FlameDetector.cpp \
    TargetExtractor.cpp \
    Config.cpp \
    videowork.cpp

#INCLUDEPATH += $$PWD/../opencv/build/include
INCLUDEPATH += "D:/OpenCV/OpenCV2411/build/include"
#debug:
#win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc10/lib/ -lopencv_core2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_core2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_highgui2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_imgproc2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_calib3d2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_contrib2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_core2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_features2d2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_flann2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_gpu2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_legacy2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_ml2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_nonfree2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_objdetect2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_ocl2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_photo2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_stitching2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_superres2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_video2411d
win32:CONFIG(debug, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_videostab2411d
#release:
#win32:CONFIG(release, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc10/lib/ -lopencv_core2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_core2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_highgui2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_imgproc2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_calib3d2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_contrib2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_core2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_features2d2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_flann2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_gpu2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_legacy2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_ml2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_nonfree2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_objdetect2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_ocl2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_photo2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_stitching2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_superres2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_video2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc10/lib/" -lopencv_videostab2411

HEADERS += \
    common.h \
    utils.h \
    VideoHandler.h \
    FeatureAnalyzer.h \
    FlameDecider.h \
    FlameDetector.h \
    TargetExtractor.h \
    Config.h \
    videowork.h
