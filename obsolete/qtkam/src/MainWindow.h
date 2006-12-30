#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <kmainwindow.h>

#define MESSAGE_TIME 5000

/* Forward declarations */
class KIconView;
class KMenuBar;
class KAction;
class KPopupMenu;

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
    KAction *saveAction, *deleteAction, *quitAction, /* *selectWorkDirAction,*/
            *downloadThumbsAction, *selectAllAction, *invertSelectionAction,
            *clearSelectionAction, *selectCameraAction, *initCameraAction, 
            *configureCameraAction,*cameraInformationAction, 
            *cameraManualAction, *cameraAboutAction;
    
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
