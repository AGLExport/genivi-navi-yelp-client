#include <QApplication>
#include <iostream>
#include "MainApp.h"
#include <getopt.h>

#include <libhomescreen.hpp>
#include <qlibwindowmanager.h>


#define DEFAULT_CREDENTIALS_FILE "/etc/poikey"

using namespace std;

QLibWindowmanager* qwm;
LibHomeScreen* hs;
QString myname;
MainApp *mainapp;

void SyncDrawHandler(json_object *object)
{
	qwm->endDraw(myname);
}

void TapShortcutHandler(json_object *object)
{
    cerr << "activateWindow " << myname.toStdString().c_str() << endl;
    qwm->activateWindow(myname);
}

int main(int argc, char *argv[], char *env[])
{
    int opt;
    QApplication a(argc, argv);
    QString credentialsFile(DEFAULT_CREDENTIALS_FILE);
    qwm = new QLibWindowmanager();
    hs = new LibHomeScreen();
	myname = QString("POI");
	
	QString pt = QString(argv[1]);
	int port = pt.toInt();
	QString secret = QString(argv[2]);
	std::string token = secret.toStdString();

    if (qwm->init(port, secret) != 0) {
        exit(EXIT_FAILURE);
    }

    if (qwm->requestSurface(myname) != 0) {
        cerr << "Error: wm check failed" << endl;
        exit(EXIT_FAILURE);
    }

	qwm->set_event_handler(QLibWindowmanager::Event_SyncDraw, SyncDrawHandler);

    mainapp = new MainApp();

	hs->init(port, token.c_str());
	
	hs->set_event_handler(LibHomeScreen::Event_TapShortcut, TapShortcutHandler);

    //force setting
    mainapp->setInfoScreen(true);
    mainapp->setKeyboard(true);

    /* check naviapi */
    if (mainapp->CheckNaviApi(argc, argv) == false)
    {
        cerr << "Error: naviapi check failed" << endl;
        return -1;
    }

    /* then, authenticate connexion to POI service: */
    if (mainapp->AuthenticatePOI(credentialsFile) < 0)
    {
        cerr << "Error: POI server authentication failed" << endl;
        return -1;
    }

    cerr << "authentication succes !" << endl;

    /* now, let's start monitor user inut (register callbacks): */
    if (mainapp->StartMonitoringUserInput() < 0)
        return -1;

	qwm->activateWindow(myname);

    /* main loop: */
    return a.exec();
}
