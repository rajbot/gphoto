#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <kmainwindow.h>
//#include <kmenubar.h>
//#include <kiconview.h>

#include <qpopupmenu.h>

/* Forward declarations */
class KIconView;
class KMenuBar;

/* MainWindow class */
class MainWindow : public KMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

protected:
    void initWidgets();

private:
    KMenuBar *menu;
    KPopupMenu *fileMenu, *editMenu, *commandMenu, *cameraMenu, *help;
    KIconView *iconView;
    enum { InitCameraID, DownloadThumbsID,  SaveSelectedID,
           DeleteSelectedID };
    enum { DownloadThumbsMenuID, DeleteSelectedMenuID, SaveSelectedMenuID };

private slots:
    void initCamera();
    void downloadThumbs();
    void saveSelected();
    void selectWorkDir();
    void selectAll();
    void selectInverse();
    void selectNone();
    void selectCamera();
    void selectionChanged();
    void deleteSelected();
    void configureCamera();
    void cameraInformation();
    void cameraManual();
    void cameraAbout();
};

#endif
