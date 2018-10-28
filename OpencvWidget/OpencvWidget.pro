#-------------------------------------------------
#
# Project created by QtCreator 2018-10-25T09:11:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpencvWidget
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        mainwindow.h \
    documentdetector.h

FORMS += \
        mainwindow.ui
INCLUDEPATH += "D:/OpenCV/OpenCV2411/build/include"
#debug:
#win32:CONFIG(debug, debug|release):LIBS += -L$$PWD/../opencv/build/x86/vc12/lib/ -lopencv_core2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_core2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_highgui2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_imgproc2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_calib3d2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_contrib2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_core2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_features2d2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_flann2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_gpu2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_legacy2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_ml2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_nonfree2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_objdetect2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_ocl2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_photo2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_stitching2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_superres2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_video2411d
win32:CONFIG(debug, debug|release):LIBS +=   -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_videostab2411d
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_core2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_highgui2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_imgproc2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_calib3d2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_contrib2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_core2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_features2d2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_flann2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_gpu2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_legacy2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_ml2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_nonfree2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_objdetect2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_ocl2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_photo2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_stitching2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_superres2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_video2411
win32:CONFIG(release, debug|release):LIBS += -L"D:/OpenCV/OpenCV2411/build/x86/vc12/lib/" -lopencv_videostab2411
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
