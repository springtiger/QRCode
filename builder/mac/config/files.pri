#headers
files = $$files($$PROJECTDIR/src/*.h*, true)
for (f, files) {
    BASENAME = $$basename(f)
    WIN_FILE = $$find(BASENAME, "_win")
    UNIX_FILE = $$find(BASENAME, "_unix")
    UNIT_TEST = $$find(BASENAME, "UnitTest")
    !isEmpty(WIN_FILE){
        win32:HEADERS += $$f
    }
    else:!isEmpty(UNIX_FILE) {
        !win32:HEADERS += $$f
    }
    else:isEmpty(UNIT_TEST){
        HEADERS += $$f
    }
}

#source
files = $$files($$PROJECTDIR/src/*.c*, true)
for (f, files) {
    BASENAME = $$basename(f)
    WIN_FILE = $$find(BASENAME, "_win")
    UNIX_FILE = $$find(BASENAME, "_unix")
    UNIT_TEST = $$find(BASENAME, "UnitTest")
    !isEmpty(WIN_FILE){
        win32:SOURCES += $$f
    }
    else:!isEmpty(UNIX_FILE) {
        !win32:SOURCES += $$f
    }
    else:isEmpty(UNIT_TEST){
        SOURCES += $$f
    }
}

# mm files in osx
macx{
	files = $$files($$PROJECTDIR/src/*.mm, true)
	for (f, files) {
	    SOURCES += $$f
	}
}

#forms
files = $$files($$PROJECTDIR/dialogs/*.ui, true)
for (f, files) {
    FORMS += $$f
}

#resource
files = $$files($$PROJECTDIR/*.qrc, true)
for (f, files) {
    RESOURCES += $$f
}
