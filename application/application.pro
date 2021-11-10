TARGET = ste
CONFIG += release
QT += network \
    webkit
RESOURCES = application.qrc
FORMS += configdialog.ui \
    filesdialog.ui \
    searchbar.ui \
    shortcutbar.ui \
    explorerbar.ui \
    wordformatbar.ui \
    toolmanagerdialog.ui \
    searchprogressdialog.ui \
    previewmanagerdialog.ui \
    previewers/dictview.ui \
    wrapdialog.ui \
    symbolsbar.ui \
    numbersbar.ui
HEADERS = mainwindow.h \
    editwindow.h \
    stylemanager.h \
    3rdparty/qtsingleapplication/qtsinglecoreapplication.h \
    3rdparty/qtsingleapplication/qtsingleapplication.h \
    3rdparty/qtsingleapplication/qtlockedfile.h \
    3rdparty/qtsingleapplication/qtlocalpeer.h \
    clicklabel.h \
    config.h \
    configdialog.h \
    recentfiles.h \
    searchbar.h \
    3rdparty/utf/Utf8_16.h \
    editstyle.h \
    filesdialog.h \
    formatmanager.h \
    3rdparty/uncrustify/src/uncrustify_version.h \
    3rdparty/uncrustify/src/uncrustify_types.h \
    3rdparty/uncrustify/src/unc_ctype.h \
    3rdparty/uncrustify/src/token_names.h \
    3rdparty/uncrustify/src/token_enum.h \
    3rdparty/uncrustify/src/punctuators.h \
    3rdparty/uncrustify/src/prototypes.h \
    3rdparty/uncrustify/src/options.h \
    3rdparty/uncrustify/src/md5.h \
    3rdparty/uncrustify/src/logmask.h \
    3rdparty/uncrustify/src/logger.h \
    3rdparty/uncrustify/src/log_levels.h \
    3rdparty/uncrustify/src/ListManager.h \
    3rdparty/uncrustify/src/ChunkStack.h \
    3rdparty/uncrustify/src/chunk_list.h \
    3rdparty/uncrustify/src/char_table.h \
    3rdparty/uncrustify/src/base_types.h \
    3rdparty/uncrustify/src/backup.h \
    3rdparty/uncrustify/src/args.h \
    3rdparty/uncrustify/src/align_stack.h \
    3rdparty/uncrustify/win32/windows_compat.h \
    utils.h \
    sidebarbase.h \
    shortcutbar.h \
    explorerbar.h \
    wordformatbar.h \
    editdefines.h \
    defines.h \
    toolmanagerdialog.h \
    lexers/extlexerasm.h \
    lexers/lexerdsl.h \
    searchprogressdialog.h \
    previewmanager.h \
    previewmanagerdialog.h \
    previewers/dslrender.h \
    previewers/abstractdict.h \
    previewers/htmlrender.h \
    previewers/dictview.h \
    wrapdialog.h \
    previewers/xdxfrender.h \
    numbersbar.h \
    translationmanager.h \
    toolmanager.h \
    tabwidget.h \
    config.h \
    utilsui.h \
    symbolsbar.h
SOURCES = main.cpp \
    mainwindow.cpp \
    editwindow.cpp \
    stylemanager.cpp \
    3rdparty/qtsingleapplication/qtsinglecoreapplication.cpp \
    3rdparty/qtsingleapplication/qtsingleapplication.cpp \
    3rdparty/qtsingleapplication/qtlockedfile.cpp \
    3rdparty/qtsingleapplication/qtlocalpeer.cpp \
    mainio.cpp \
    mainconfig.cpp \
    clicklabel.cpp \
    config.cpp \
    configdialog.cpp \
    recentfiles.cpp \
    mainprint.cpp \
    mainbookmark.cpp \
    searchbar.cpp \
    mainsearch.cpp \
    3rdparty/utf/Utf8_16.cxx \
    editstyle.cpp \
    filesdialog.cpp \
    mainformat.cpp \
    formatmanager.cpp \
    3rdparty/uncrustify/src/width.cpp \
    3rdparty/uncrustify/src/universalindentgui.cpp \
    3rdparty/uncrustify/src/uncrustify.cpp \
    3rdparty/uncrustify/src/tokenize_cleanup.cpp \
    3rdparty/uncrustify/src/tokenize.cpp \
    3rdparty/uncrustify/src/space.cpp \
    3rdparty/uncrustify/src/sorting.cpp \
    3rdparty/uncrustify/src/semicolons.cpp \
    3rdparty/uncrustify/src/punctuators.cpp \
    3rdparty/uncrustify/src/parse_frame.cpp \
    3rdparty/uncrustify/src/parens.cpp \
    3rdparty/uncrustify/src/output.cpp \
    3rdparty/uncrustify/src/options.cpp \
    3rdparty/uncrustify/src/newlines.cpp \
    3rdparty/uncrustify/src/md5.cpp \
    3rdparty/uncrustify/src/logmask.cpp \
    3rdparty/uncrustify/src/logger.cpp \
    3rdparty/uncrustify/src/lang_pawn.cpp \
    3rdparty/uncrustify/src/keywords.cpp \
    3rdparty/uncrustify/src/indent.cpp \
    3rdparty/uncrustify/src/detect.cpp \
    3rdparty/uncrustify/src/defines.cpp \
    3rdparty/uncrustify/src/d.tokenize.cpp \
    3rdparty/uncrustify/src/combine.cpp \
    3rdparty/uncrustify/src/ChunkStack.cpp \
    3rdparty/uncrustify/src/chunk_list.cpp \
    3rdparty/uncrustify/src/braces.cpp \
    3rdparty/uncrustify/src/brace_cleanup.cpp \
    3rdparty/uncrustify/src/backup.cpp \
    3rdparty/uncrustify/src/args.cpp \
    3rdparty/uncrustify/src/align_stack.cpp \
    3rdparty/uncrustify/src/align.cpp \
    utils.cpp \
    maintrimlines.cpp \
    sidebarbase.cpp \
    shortcutbar.cpp \
    mainshortcuts.cpp \
    explorerbar.cpp \
    mainexplorer.cpp \
    wordformatbar.cpp \
    mainwordformat.cpp \
    mainsession.cpp \
    toolmanager.cpp \
    tabwidget.cpp \
    translationmanager.cpp \
    toolmanagerdialog.cpp \
    lexers/extlexerasm.cpp \
    lexers/lexerdsl.cpp \
    searchprogressdialog.cpp \
    previewmanager.cpp \
    previewmanagerdialog.cpp \
    previewers/dslrender.cpp \
    previewers/abstractdict.cpp \
    previewers/htmlrender.cpp \
    previewers/dictview.cpp \
    autotag.cpp \
    wrapdialog.cpp \
    symbolsbar.cpp \
    mainsymbols.cpp \
    previewers/xdxfrender.cpp \
    numbersbar.cpp \
    mainnumbers.cpp \
    utilsui.cpp
INCLUDEPATH += 3rdparty
INCLUDEPATH += 3rdparty/uncrustify/win32
QSCINTILLA2_ROOT = 3rdparty/qscintilla
INCLUDEPATH += $$QSCINTILLA2_ROOT/Qt4
INCLUDEPATH += $$QSCINTILLA2_ROOT/include
LIBS += -L$$QSCINTILLA2_ROOT/lib
win32: {
    ICONV_ROOT = 3rdparty/iconv/win32
    ENCA_ROOT = 3rdparty/enca/win32
    TARGET = ../../bin/ste
    SOURCES += 3rdparty/qtsingleapplication/qtlockedfile_win.cpp
    LIBS += -L$$ICONV_ROOT/lib \
        -liconv \
        -L$$ENCA_ROOT/lib
    INCLUDEPATH += $$ICONV_ROOT/include
    INCLUDEPATH += $$ENCA_ROOT/include
    RC_FILE = ste.rc
    DEFINES += _WIN32
}
unix: {
    DEFINES += HAVE_STDINT_H
    SOURCES += 3rdparty/qtsingleapplication/qtlockedfile_unix.cpp
    LIBS += -lcxxtools
    TARGET = ../bin/ste
    target.path = /usr/bin
    isEmpty(target.path):target.path = /usr/local/bin
    INSTALLS += target
    data.path = /usr/share/sintegrial/ste
    data.files = ../bin/files
    INSTALLS += data
}
LIBS += -lqscintilla2 \
    -lenca
OTHER_FILES += 3rdparty/uncrustify/src/Makefile.in \
    3rdparty/uncrustify/src/Makefile.am \
    3rdparty/uncrustify/src/config.h.in \
    application.pro
TRANSLATIONS = lang/ste_ru.ts \
    lang/ste_cz.ts
