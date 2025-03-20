QT += core gui widgets
CONFIG += c++17 windows #console | windows
RC_ICONS = Key.ico

# Include path and libraries (vcpkg OpenSSL)
INCLUDEPATH += C:/vcpkg/installed/x64-windows/include
LIBS += -LC:/vcpkg/installed/x64-windows/lib -llibcrypto -llibssl

# Sources
SOURCES += main.cpp \
           crypt.cpp \
           passwordmanager.cpp

# Headers
HEADERS += crypt.h \
           passwordmanager.h

