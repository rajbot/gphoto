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
#include <qpixmap.h>
#include <qiconview.h>
#include <qwidget.h>
#include <qfiledialog.h>
#include <qprogressbar.h>

#include "MainWindow.h"
#include "MainWindow.moc"

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
    else 
        statusBar()->message(i18n("Camera not ready"));
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
    iconView->setItemsMovable(false);
    iconView->setResizeMode(KIconView::Adjust);
    connect(iconView,SIGNAL(selectionChanged()),
            this,SLOT(selectionChanged()));
    
    /* FIXME: Iconviews don't support background pixmaps ? */
    /*iconView->setBackgroundPixmap( KApplication::kApplication()->iconLoader()->loadIcon("canvas",KIcon::Desktop));
    iconView->setBackgroundMode(QWidget::FixedPixmap);*/
    setCentralWidget(iconView);
                      
    /* File menu */
    fileMenu = new KPopupMenu();
    fileMenu->insertItem(i18n("&Save Selected Photos"),
                         this, SLOT(saveSelected()),
                         CTRL + Key_S, SaveSelectedMenuID);
    fileMenu->setItemEnabled(SaveSelectedMenuID,false);
    fileMenu->insertSeparator();
    fileMenu->insertItem(i18n("Set &Working Directory"),
                         this, SLOT(selectWorkDir()), CTRL + Key_W);
    fileMenu->insertSeparator();
    fileMenu->insertItem(i18n("&Exit"),
                         this, SLOT(close()), CTRL + Key_X);

    /* Edit menu */
    editMenu = new KPopupMenu();
    editMenu->insertItem(i18n("Select &All"), this, SLOT(selectAll()), 
                         SHIFT + Key_A);
    editMenu->insertItem(i18n("&Invert Selection"), 
                         this, SLOT(selectInverse()), SHIFT + Key_I);
    editMenu->insertItem(i18n("&Clear Selection"), 
                         this, SLOT(selectNone()), SHIFT + Key_N);
    
    /* Command menu */
    commandMenu = new KPopupMenu();
    commandMenu->insertItem(i18n("Download Thumbnails"), this, 
                            SLOT(downloadThumbs()), CTRL + Key_T,
                            DownloadThumbsMenuID);
    commandMenu->setItemEnabled(DownloadThumbsMenuID,false);
    commandMenu->insertSeparator();
    commandMenu->insertItem(i18n("Delete Selected"), 
                            this, SLOT(deleteSelected()), 
                            CTRL + Key_D, DeleteSelectedMenuID);
    commandMenu->setItemEnabled(DeleteSelectedMenuID,false);
    
    /* Camera menu */
    cameraMenu = new KPopupMenu();
    cameraMenu->insertItem(i18n("Select Camera"), this, SLOT(selectCamera()), 
                           CTRL + Key_C);
    cameraMenu->insertSeparator();
    cameraMenu->insertItem(i18n("&Configure"), this, SLOT(configureCamera()));
    cameraMenu->insertItem(i18n("&Information"), this, 
                           SLOT(cameraInformation()));
    cameraMenu->insertItem(i18n("&Manual"), this, SLOT(cameraManual()));
    cameraMenu->insertItem(i18n("&About the driver"), this, 
                           SLOT(cameraAbout()));
    
    /* Help menu */
    help = helpMenu();
    
    /* Menu Bar */
    menuBar()->insertItem(i18n("&File"), fileMenu);
    menuBar()->insertItem(i18n("&Edit"), editMenu);
    menuBar()->insertItem(i18n("&Command"),commandMenu);
    menuBar()->insertItem(i18n("C&amera"),cameraMenu);
    menuBar()->insertItem(i18n("&Help"), help);
   
    toolBar()->insertButton("connect_creating", InitCameraID, SIGNAL(clicked()),
                            this, SLOT(initCamera()), true,
                            i18n("Initialize Camera"));
    toolBar()->insertButton("queue", DownloadThumbsID, SIGNAL(clicked()),
                            this, SLOT(downloadThumbs()), false,
                            i18n("Download thumbnails"));
    toolBar()->insertButton("filesave", SaveSelectedID, SIGNAL(clicked()),
                            this, SLOT(saveSelected()), false, 
                            i18n("Save Selected Pictures")); 
    toolBar()->insertButton("edittrash", DeleteSelectedID, SIGNAL(clicked()),
                            this, SLOT(deleteSelected()), false,
                            i18n("Delete Selected Pictures"));
}

void MainWindow::initCamera()
{
    /* Clear thumbnail overview */
    iconView->clear();

    try {
        /* Initialize Camera */
        GPInterface::initCamera();
        statusBar()->message(i18n("Camera ready"));

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
        statusBar()->message(i18n("Camera not ready"));
    } 
}

void MainWindow::saveSelected() 
{
    for (QIconViewItem *i = iconView->firstItem(); i; i = i->nextItem()) {
        if (i->isSelected())
            GPInterface::downloadPicture(i->text(),"/");
    } 
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
    iconView->clear();
    try {
        QProgressBar* bar=new QProgressBar(statusBar());
        statusBar()->addWidget(bar);
        GPInterface::downloadThumbs(iconView);
    }
    catch (QString str) {
        KMessageBox::error(this, str);
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
    if (KMessageBox::questionYesNo(this, 
        "Are you sure you want to delete the selected pictures ?", 
        "Delete") == KMessageBox::Yes) {
        try {
        for (QIconViewItem *i = iconView->firstItem();i; i=i->nextItem()) {
            if (i->isSelected()) {
                GPInterface::deletePicture(i->text(),"/");
                delete i;
                iconView->arrangeItemsInGrid();
            }
        }
        } catch (QString err) { KMessageBox::error(this, err); }
    }
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
