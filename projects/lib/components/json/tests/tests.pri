win32:config += CONSOLE

mac {
	CONFIG -= app_bundle
}

QT = core testlib
DEFINES += LIB_EXPORT=""

include(../src/json.pri)

OBJECTS_DIR = .obj
MOC_DIR = .moc
