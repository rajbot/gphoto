#include <kapp.h>
#include <kpopupmenu.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kmessagebox.h>
#include <kiconview.h>
#include <kimageio.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <ktoolbarbutton.h>
#include <klocale.h>
#include <kaction.h>
#include <kstdaction.h>
#include <qpixmap.h>
#include <qprogressdialog.h>
#include <qiconview.h>
#include <qwidget.h>
#include <qfiledialog.h>

#include "MainWindow.h"
#include "MainWindow.moc"

#include "GPInterface.h"
#include "GPMessenger.h"
#include "SelectCameraDialog.h"

MainWindow::MainWindow() : KMainWindow() 
{
    /* Draw main window */
    initWidgets();
    
    /* Load JPEG support from KDE library */
    KImageIO::registerFormats();
     
    /* Initialize interface & gPhoto2 */
    GPInterface::initialize();
    
    /* Set window size */
    setGeometry(GPInterface::getGeometry());
        
    /* Check if a camera is configured */
    if (GPInterface::getCamera().isNull())
        selectCamera();
     
    statusBar()->message(i18n("Ready"));
}


MainWindow::~MainWindow() 
{
    GPInterface::setGeometry(geometry());
    GPInterface::shutdown();
}


/* 
 * Draws & initializes all the widgets of the main window 
 */
void MainWindow::initWidgets() 
{
    setPlainCaption("QtKam");

    /* Construct Actions */
    saveAction = KStdAction::save(this, SLOT(saveSelected()),this); 
    quitAction = KStdAction::quit(this, SLOT(close()),this);
    deleteAction = new KAction(i18n("Delete"), "edittrash",
                    Key_Delete, this, SLOT(deleteSelected()), this);
    downloadThumbsAction = new KAction(i18n("Download Thumbs"), "queue",
                    CTRL + Key_T, this, SLOT(downloadThumbs()), this);
    selectAllAction = new KAction(i18n("Select &All"), SHIFT + Key_A,
                    this, SLOT(selectAll()), this);      
    invertSelectionAction = new KAction(i18n("&Invert Selection"),
                    SHIFT + Key_I, this, SLOT(selectInverse()), this); 
    clearSelectionAction = new KAction(i18n("&Clear Selection"),
                    SHIFT + Key_N, this, SLOT(selectNone()), this);
    selectCameraAction = new KAction(i18n("Select &Camera"),  
                    CTRL + Key_C, this, SLOT(selectCamera()), this);
    initCameraAction = new KAction(i18n("Reset Camera"),
                    "connect_creating", CTRL + Key_R, this, 
                    SLOT(initCamera()), this);
    configureCameraAction = new KAction(i18n("&Configure"), 
                    0, this, SLOT(configureCamera()), this);
    cameraInformationAction = new KAction(i18n("&Information"),
                    0, this, SLOT(cameraInformation()), this);
    cameraManualAction = new KAction(i18n("&Manual"), 
                    0, this, SLOT(cameraManual()), this);
    cameraAboutAction = new KAction(i18n("&About the driver"),
                    0, this, SLOT(cameraAbout()), this);
                 
    /* Initialize actions */
    saveAction->setEnabled(false);
    deleteAction->setEnabled(false);

    /* Create & initialize icon view */
    iconView = new KIconView(this);
    iconView->setMode(KIconView::Select);
    iconView->setSelectionMode(KIconView::Multi);
    iconView->setItemsMovable(false);
    iconView->setResizeMode(KIconView::Adjust);
    connect(iconView,SIGNAL(selectionChanged()),
            this,SLOT(selectionChanged()));
    setCentralWidget(iconView);
    
    /* Create file menu */
    fileMenu = new KPopupMenu();
    saveAction->plug(fileMenu);
    deleteAction->plug(fileMenu);
    fileMenu->insertSeparator();
    quitAction->plug(fileMenu);

    /* Create edit menu */
    editMenu = new KPopupMenu();
    selectAllAction->plug(editMenu);
    invertSelectionAction->plug(editMenu); 
    clearSelectionAction->plug(editMenu);
    
    /* Create command menu */
    commandMenu = new KPopupMenu();
    initCameraAction->plug(commandMenu);
    downloadThumbsAction->plug(commandMenu);
    
    /* Camera menu */
    cameraMenu = new KPopupMenu();
    selectCameraAction->plug(cameraMenu);
    cameraMenu->insertSeparator();
    configureCameraAction->plug(cameraMenu);
    cameraInformationAction->plug(cameraMenu);
    cameraManualAction->plug(cameraMenu);
    cameraAboutAction->plug(cameraMenu);

    /* Help menu */
    help = helpMenu();
    
    /* Menu Bar */
    menuBar()->insertItem(i18n("&File"), fileMenu);
    menuBar()->insertItem(i18n("&Edit"), editMenu);
    menuBar()->insertItem(i18n("&Command"),commandMenu);
    menuBar()->insertItem(i18n("C&amera"),cameraMenu);
    menuBar()->insertItem(i18n("&Help"), help);
   
    /* Create toolbar */
    downloadThumbsAction->plug(toolBar());
    saveAction->plug(toolBar());
    deleteAction->plug(toolBar());
}


void MainWindow::initCamera()
{
    /* Clear thumbnail overview */
    iconView->clear();

    statusBar()->message(i18n("Initializing camera ..."));
    try {
        /* Initialize Camera */
        GPInterface::initCamera();
        //statusBar()->message(i18n("Camera ready"));

        /* Enable downloading of thumbs */
        downloadThumbsAction->setEnabled(true);

        /* Change window title */
        setCaption(GPInterface::getCamera());
        statusBar()->message(i18n("Done"),MESSAGE_TIME);
    }
    catch (QString msg) {
        /* Disable downloading of thumbs */
        downloadThumbsAction->setEnabled(false);

        /* Change window title */
        setPlainCaption("QtKam");
        KMessageBox::error(this, msg);
        statusBar()->message(i18n("Error"), MESSAGE_TIME);
    } 
}


/**
 * Saves the selected pictures.
 * First pops up a dialog box asking for the target dir. Then pops up 
 * a progress dialog for feedback.
 */
void MainWindow::saveSelected() 
{
    /* Select target */
    selectWorkDir();

    /* Create a progress dialog */
    QProgressDialog* progress = new QProgressDialog(
        i18n("Downloading ..."), i18n("Cancel"), 100, this, "progress", TRUE);
    progress->setCaption("Download");
    connect(GPMessenger::instance(),SIGNAL(progressChanged(int)),
            progress,SLOT(setProgress(int)));
    progress->setAutoReset(false);
    progress->setAutoClose(false); 
    progress->setProgress(0);
    KApplication::kApplication()->processEvents();
     
    /* Download all pictures */
    for (QIconViewItem *i = iconView->firstItem(); 
                    i && !progress->wasCancelled(); i = i->nextItem() ) {
        if (i->isSelected()) {
            /* Update progress dialog */
            progress->setLabelText(i18n("Downloading ") + i->text() + "...");

            /* Download picture */
            GPInterface::downloadPicture(i->text(),"/");
        }
    } 

    progress->setProgress(100);
    delete progress;
}


void MainWindow::selectWorkDir()
{
    QString dir = KFileDialog::getExistingDirectory(
        GPInterface::getWorkDir(), this);
    
    if (!dir.isNull()) 
        GPInterface::setWorkDir(dir);
}


void MainWindow::downloadThumbs()
{
    /* Check if camera has to be initialized */
    if (!GPInterface::isInitialized())
        initCamera();
    
    /* Clear iconview */
    iconView->clear();

    /* Try downloading thumbs */
    statusBar()->message("Downloading thumbs ...");
    try {
        GPInterface::downloadThumbs(iconView);
        statusBar()->message("Done",MESSAGE_TIME);
    }
    catch (QString str) {
        KMessageBox::error(this, str);
        statusBar()->message("Error",MESSAGE_TIME);
    }
}


/*
 * Selection functions. 
 */
void MainWindow::selectAll() 
    { iconView->selectAll(true); }

void MainWindow::selectNone()
    { iconView->clearSelection(); }

void MainWindow::selectInverse()
    { iconView->invertSelection(); }


/**
 * Pops up the camera selection dialog.
 */
void MainWindow::selectCamera()
{
    /* Popup the 'select camera' dialog */
    SelectCameraDialog* w = new SelectCameraDialog(this);
    w->exec();
}


/**
 * Notify that the selection has changed.
 * Will check if there are still items selected, and will enable
 * actions based on the result. 
 */
void MainWindow::selectionChanged()
{
    /* Check if there are items selected */
    bool selected = false;
    for (QIconViewItem *i = iconView->firstItem(); i; i = i->nextItem())
        selected = selected || i->isSelected();
        
    saveAction->setEnabled(selected);
    deleteAction->setEnabled(selected);
}


/**
 * Deletes the selected files (after asking for confirmation).
 */
void MainWindow::deleteSelected()
{
    if (KMessageBox::questionYesNo(this, 
        i18n("Are you sure you want to delete the selected pictures ?"), 
        i18n("Delete")) == KMessageBox::Yes) {

        try {
        for (QIconViewItem *i = iconView->firstItem();i; i=i->nextItem()) {
            if (i->isSelected()) {
                statusBar()->message(i18n("Deleting ") + i->text() + " ...");
                GPInterface::deletePicture(i->text(),"/");
                delete i;
                iconView->arrangeItemsInGrid();
            }
        }
        statusBar()->message(i18n("Done"));
        } catch (QString err) { 
            KMessageBox::error(this, err); 
            statusBar()->message(i18n("Error"), MESSAGE_TIME);
        }
    }
}


/**
 * Displays a message box with all the information of the camera
 */ 
void MainWindow::cameraInformation()
{
    KMessageBox::information(this, GPInterface::getInformation(), 
                             "Camera Information");
}


void MainWindow::configureCamera()
{
}


void MainWindow::cameraManual()
{
    KMessageBox::information(this, GPInterface::getManual(),
                            "Camera Manual");
}

void MainWindow::cameraAbout()
{
    KMessageBox::information(this, GPInterface::getAbout(),
                             "About the driver");
}
