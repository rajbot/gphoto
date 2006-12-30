#include <kapp.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "MainWindow.h"

/* Program info */
static const char *description = "Qt/KDE interface to gphoto2";
static const char *version = "v0.0.1";

/* Main program */
int main( int argc, char **argv )
{
    /* About data */
    KAboutData aboutData( "qtkam", "QtKam", version, description, 
         KAboutData::License_GPL, "(c) GPhoto2 team", 0, 
         "http://www.gphoto2.org",
         "gphoto@gphoto.net");
    aboutData.addAuthor("Remko Troncon",0, 
                        "spike@kotnet.org",
                        "http://spike.studentenweb.org/");
  
    /* Initialize application */ 
    KCmdLineArgs::init( argc, argv, &aboutData );
    KApplication app;

    MainWindow *mainwindow = new MainWindow();
    app.setTopWidget(mainwindow);
    mainwindow->show();
    
    /* Start application */
    return app.exec();
}
