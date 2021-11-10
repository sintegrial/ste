#include <qtsingleapplication/qtsingleapplication.h>

#include "mainwindow.h"
#include "translationmanager.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(application);

    QtSingleApplication app(argc, argv);

    // check single run
    QString params = app.arguments().join("\n");
    if (app.sendMessage(params))
      return 0;

    // translation - should be done before startup
    INIT_SETTINGS;
    QString lang = settings.value("language", "en").toString();
    TranslationManager *tm = new TranslationManager(&app);
    tm->translate(lang);

    // startup
    MainWindow mainWin;
    mainWin.start();

    app.setActivationWindow(&mainWin);

    QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
                     &mainWin, SLOT(messageParams(const QString&)));

    app.activateWindow();

    return app.exec();
}
