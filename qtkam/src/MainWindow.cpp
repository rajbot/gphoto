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
#include <qpixmap.h>
#include <qiconview.h>
#include <qwidget.h>
#include <qfiledialog.h>

#include "MainWindow.h"
#include "GPInterface.h"
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

    statusBar()->message("Camera not ready");
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

    /* Icon view */
    iconView = new KIconView(this);
    iconView->setMode(KIconView::Select);
    iconView->setSelectionMode(KIconView::Multi);
    connect(iconView,SIGNAL(selectionChanged()),
            this,SLOT(selectionChanged()));
    
    /* FIXME: Iconviews don't support background pixmaps ? */
    /*iconView->setBackgroundPixmap( KApplication::kApplication()->iconLoader()->loadIcon("canvas",KIcon::Desktop));
    iconView->setBackgroundMode(QWidget::FixedPixmap);*/
    setCentralWidget(iconView);
                      
    /* File menu */
    fileMenu = new KPopupMenu();
    fileMenu->insertItem("&Save Selected Photos",this,SLOT(saveSelected()),
                         CTRL + Key_S, SaveSelectedMenuID);
    fileMenu->setItemEnabled(SaveSelectedMenuID,false);
    fileMenu->insertSeparator();
    fileMenu->insertItem("Set &Working Directory",this,SLOT(selectWorkDir()),
                             CTRL + Key_W);
    fileMenu->insertSeparator();
    fileMenu->insertItem("&Exit",this,SLOT(close()),
                         CTRL + Key_X);

    /* Edit menu */
    editMenu = new KPopupMenu();
    editMenu->insertItem("Select &All", this, SLOT(selectAll()), 
                          SHIFT + Key_A);
    editMenu->insertItem("&Invert Selection", this, SLOT(selectInverse()), 
                          SHIFT + Key_I);
    editMenu->insertItem("&Clear Selection", this, SLOT(selectNone()),
                          SHIFT + Key_N);
    
    /* Command menu */
    commandMenu = new KPopupMenu();
    commandMenu->insertItem("Download Thumbnails", this, 
                            SLOT(downloadThumbs()), CTRL + Key_T,
                            DownloadThumbsMenuID);
    commandMenu->setItemEnabled(DownloadThumbsMenuID,false);
    commandMenu->insertSeparator();
    commandMenu->insertItem("Delete Selected", this, SLOT(deleteSelected()), 
                           CTRL + Key_D, DeleteSelectedMenuID);
    commandMenu->setItemEnabled(DeleteSelectedMenuID,false);
    
    /* Camera menu */
    cameraMenu = new KPopupMenu();
    cameraMenu->insertItem("Select Camera", this, SLOT(selectCamera()), 
                           CTRL + Key_C);
    cameraMenu->insertSeparator();
    cameraMenu->insertItem("&Configure", this, SLOT(configureCamera()));
    cameraMenu->insertItem("&Information", this, SLOT(cameraInformation()));
    cameraMenu->insertItem("&Manual", this, SLOT(cameraManual()));
    cameraMenu->insertItem("&About the driver", this, SLOT(cameraAbout()));
    
    /* Help menu */
    help = helpMenu();
    
    /* Menu Bar */
    menuBar()->insertItem("&File", fileMenu);
    menuBar()->insertItem("&Edit", editMenu);
    menuBar()->insertItem("&Command",commandMenu);
    menuBar()->insertItem("C&amera",cameraMenu);
    menuBar()->insertItem("&Help", help);
   
    toolBar()->insertButton("", InitCameraID, SIGNAL(clicked()),
                            this, SLOT(initCamera()), true,
                            "Initialize Camera");
    toolBar()->insertButton("queue", DownloadThumbsID, SIGNAL(clicked()),
                            this, SLOT(downloadThumbs()), false,
                            "Download thumbnails");
    toolBar()->insertButton("filesave", SaveSelectedID, SIGNAL(clicked()),
                            this, SLOT(saveSelected()), false, 
                            "Save Selected Pictures"); 
    toolBar()->insertButton("edittrash", DeleteSelectedID, SIGNAL(clicked()),
                            this, SLOT(deleteSelected()), false,
                            "Delete Selected Pictures");
}

void MainWindow::initCamera()
{
    /* Clear thumbnail overview */
    iconView->clear();

    try {
        /* Initialize Camera */
        GPInterface::initCamera();
        statusBar()->message("Camera ready");

        /* Enable downloading of thumbs */
        toolBar()->setItemEnabled(DownloadThumbsID,true);
        commandMenu->setItemEnabled(DownloadThumbsMenuID,true);

        /* Change window title */
        setCaption(GPInterface::getCamera());
    }
    catch (QString msg) {
        /* Disable downloading of thumbs */
        toolBar()->setItemEnabled(DownloadThumbsID,false);
        commandMenu->setItemEnabled(DownloadThumbsMenuID,false);

        /* Change window title */
        setPlainCaption("QtKam");
        KMessageBox::error(this, msg);
        statusBar()->message("Camera not ready");
    } 
}


/* Slots */
void MainWindow::saveSelected() 
{
}


void MainWindow::selectWorkDir()
{
    QString dir = KFileDialog::getExistingDirectory(
        GPInterface::getFolder(), this);
    
    if (!dir.isNull()) 
        GPInterface::setFolder(dir);
}


void MainWindow::downloadThumbs()
{
    
    iconView->clear();
    GPInterface::downloadThumbs(iconView);
    /*new QIconViewItem(iconView,0,"Testje", 
            GPInterface::downloadThumb(0));
    new QIconViewItem(iconView,0,"Testje 2",
            GPInterface::downloadThumb(1));*/
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
 * Pops up the camera selection dialog, and initializes the
 * newly selected camera.
 */
void MainWindow::selectCamera()
{
    /* Popup the 'select camera' dialog */
    SelectCameraDialog* w = new SelectCameraDialog(this);
    w->exec();

    /* If the dialog was closed with 'ok', initialize new camera */
    if (w->result() == SelectCameraDialog::Accepted)
        initCamera();
}

void MainWindow::selectionChanged()
{
    if (iconView->count() > 0) {
        toolBar()->getButton(SaveSelectedID)->setOn(true);
        toolBar()->getButton(DeleteSelectedID)->setOn(true);
        fileMenu->setItemEnabled(SaveSelectedMenuID,true);
        commandMenu->setItemEnabled(DeleteSelectedMenuID,true);
    }
    else {
        toolBar()->getButton(SaveSelectedID)->setOn(false);
        toolBar()->getButton(DeleteSelectedID)->setOn(false);
        fileMenu->setItemEnabled(SaveSelectedMenuID,false);
        commandMenu->setItemEnabled(DeleteSelectedMenuID,false);
    }
}

void MainWindow::deleteSelected()
{
    
}

void MainWindow::configureCamera()
{
}

void MainWindow::cameraInformation()
{
}

void MainWindow::cameraManual()
{
}

void MainWindow::cameraAbout()
{
}
