#include <gphoto2.h>
#include <qdir.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qrect.h>
#include <string.h>
#include <stdlib.h>

#include "GPInterface.h"
#include "GPMessenger.h"


/** 
 * Starts & initializes gphoto, and registers the interface
 * with gphoto.
 */
void GPInterface::initialize()
{
    /* Initialize gphoto */
#ifdef DEBUG
    gp_init(1);
#else
    gp_init(0);
#endif

    /* Register interface */
    gp_frontend_register(
        GPInterface::frontend_status,
        GPInterface::frontend_progress,
        GPInterface::frontend_message,
        GPInterface::frontend_confirm,
        GPInterface::frontend_prompt); 
}


/**
 * Shuts down gphoto and cleans up everything.
 */
void GPInterface::shutdown() 
{
    gp_exit();
}


void GPInterface::initCamera()
{
    char camera[1024], port[1024], speed[1024];

    /* Mark camera as uninitialized */
    cameraInitialized = false;

    /* Retrieve camera settings */
    if (gp_setting_get("qtkam", "camera", camera) != GP_OK) 
        throw QString("You must choose a camera");
    
    gp_setting_get("qtkam", "port", port);
    gp_setting_get("qtkam", "speed", speed);

    /* Create new camera */
    if (gp_camera_new(&theCamera) != GP_OK) 
        throw QString("Error creating new camera!");
    
    /* Set camera model, port & speed settings */
    gp_camera_set_model(theCamera, camera);
    gp_camera_set_port_path(theCamera, port);
    if (strlen(speed))
        gp_camera_set_port_speed(theCamera, atoi(speed));

    /* Initialize camera */
    if (gp_camera_init(theCamera) != GP_OK) 
        throw QString("Error initializing new camera!");

    /* Mark camera as initialized */
    cameraInitialized = true;
}


void GPInterface::setCamera(const QString& camera) 
{
    gp_setting_set("qtkam","camera", (char*) camera.latin1());
}

void GPInterface::setPort(const QString& port)
{
    if (!port.isNull()) 
        gp_setting_set("qtkam","port", (char*) port.latin1());
    else
        gp_setting_set("qtkam","port", "");
}

void GPInterface::setSpeed(const QString& speed)
{
    if (!speed.isNull())
        gp_setting_set("qtkam","speed", (char*) speed.latin1());
    else
        gp_setting_set("qtkam","speed", "");
}

void GPInterface::setWorkDir(const QString& folder)
{
    if (!folder.isNull())
        gp_setting_set("qtkam","workdir", (char*) folder.latin1());
    else
        gp_setting_set("qtkam","workdir", "");
}


void GPInterface::setGeometry(const QRect& geometry)
{
    gp_setting_set("qtkam","top", (char*) QString::number(geometry.top()).latin1());
    gp_setting_set("qtkam","left", (char*) QString::number(geometry.left()).latin1());
    gp_setting_set("qtkam","width", (char*) QString::number(geometry.width()).latin1());
    gp_setting_set("qtkam","height", (char*) QString::number(geometry.height()).latin1());
}


QString GPInterface::getCamera() 
{
    char buf[1024];
    if (gp_setting_get("qtkam", "camera", buf) == GP_OK)
        return QString(buf);
    else
        return QString();
}

QString GPInterface::getPort() 
{
    char buf[1024];
    if (gp_setting_get("qtkam", "port", buf) == GP_OK)
        return QString(buf);
    else
        return QString();
}

QString GPInterface::getSpeed() 
{
    char buf[1024];
    if (gp_setting_get("qtkam", "speed", buf) == GP_OK)
        return QString(buf);
    else
        return QString();
}

QString GPInterface::getWorkDir()
{
    char buf[1024];
    if (gp_setting_get("qtkam", "workdir", buf) == GP_OK)
        return QString(buf);
    else
        return ".";
}

QRect GPInterface::getGeometry() 
{
    char buf[1024];
    
    if(gp_setting_get("qtkam","top",buf) == GP_OK) {
        QRect r;
        r.setTop(QString(buf).toInt());
        gp_setting_get("qtkam","left",buf);
        r.setLeft(QString(buf).toInt());
        gp_setting_get("qtkam","width",buf);
        r.setWidth(QString(buf).toInt());
        gp_setting_get("qtkam","height",buf);
        r.setHeight(QString(buf).toInt());
        return r;
    }
    else 
        return QRect(10,10,320,240);
}


/** 
 * Returns a string list with all the supported cameras.
 */
QStringList GPInterface::getSupportedCameras() 
{
    QStringList list;
    
    /* Get the number of cameras */
    int num_cameras;
    if ((num_cameras = gp_camera_count())<0) 
        throw QString("Can't get number of camera's!");
    
    /* Build camera list */
    char* name; /* FIXME: why can't name be an array? */
    for (int i=0; i < num_cameras; i++) {
        if (gp_camera_name(i, (const char**) &name) == GP_OK) 
            list.append(name);
        else
            list.append("ERROR");
    }

    return list;
}


/**
 * Returns a string with all the camera information.
 */
QString GPInterface::getInformation()
{
    if (!cameraInitialized) 
        initCamera();
        
    CameraText buf;
    if (gp_camera_get_summary(theCamera, &buf) != GP_OK) 
        // FIXME: do proper error handling
        return QString("ERROR");

    return QString(buf.text);
}

/**
 * Returns a string with the camera manual 
 */
QString GPInterface::getManual()
{
    if (!cameraInitialized)
        initCamera();

    CameraText buf;
    if (gp_camera_get_manual(theCamera, &buf) != GP_OK)
        // FIXME: do proper error handling
        return QString("ERROR");

    return QString(buf.text);
}

/**
 * Returns a string with the driver information.
 */
QString GPInterface::getAbout()
{
    if (!cameraInitialized)
        initCamera();

    CameraText buf;
    if (gp_camera_get_about(theCamera, &buf) != GP_OK)
        // FIXME: do proper error handling
        return QString("ERROR");

    return QString(buf.text);
}


QStringList GPInterface::getSupportedPorts(const QString& camera) 
{
    QStringList list;
    CameraAbilities a;
    int num_ports;

    /* Retrieve camera abilities */
    if (gp_camera_abilities_by_name(camera.latin1(),&a)!=GP_OK) 
        throw QString("Cannot get abilities for camera");

    /* Retrieve numer of ports */
    if ((num_ports = gp_port_count_get()) < 0) 
        throw QString("Cannot get number of ports");

    /* Populate port list */
    CameraPortInfo info;
    for (int i=0 ; i < num_ports; i++) {
        if (gp_port_info_get(i, &info) == GP_OK) {
            /* Check if port is supported */
            if (((info.type == GP_PORT_SERIAL) && SERIAL_SUPPORTED(a.port))
                || ((info.type == GP_PORT_PARALLEL) &&
                    PARALLEL_SUPPORTED(a.port))
                || ((info.type == GP_PORT_IEEE1394) &&
                    IEEE1394_SUPPORTED(a.port))
                || ((info.type == GP_PORT_NETWORK) &&
                    NETWORK_SUPPORTED(a.port))
                || ((info.type == GP_PORT_USB) && USB_SUPPORTED(a.port))) {
                /* Add port to list */
                //port_combo->insertItem(QString(info.name) + " (" + ...
                list.append(info.path);
            }
        }
    }
    
    return list;
}


QStringList GPInterface::getSupportedSpeeds(const QString& camera)
{
    QStringList list;
    CameraAbilities a;

    /* Retrieve camera abilities */
    if (gp_camera_abilities_by_name(camera.latin1(),&a)!=GP_OK) 
        throw QString("Cannot get abilities for camera");

    /* Populate speed list */
    for (int i = 0; a.speed[i] != 0; i++)
        list.append(QString::number(a.speed[i]));

    return list;
}


void GPInterface::downloadPicture(QString file, QString folder)
{
    CameraFile *f;

    /* Initialize file */
    gp_file_new(&f);

    /* Try downloading thumb */
    if (gp_camera_file_get(theCamera, folder, file.latin1(),
                           GP_FILE_TYPE_NORMAL, f) != GP_OK)
        throw QString("Couldn't get thumb ") + file +
              QString(" in folder ") + QString(folder);

    /* Save file */
    gp_file_save(f, (char*) (getWorkDir() + file).latin1());

    /* Release file */
    gp_file_free(f);
}


void GPInterface::deletePicture(QString file, QString folder)
{
    if (gp_camera_file_delete(theCamera, folder, file.latin1()) != GP_OK)
        throw QString("Error while deleting") + file;
}


void GPInterface::downloadThumbs(QIconView* iconView)
{
    int count;
    const char* name;
    CameraList list;
     
    if (!cameraInitialized)
        throw QString("Camera not initialized");

    /* Get list of files */
    if (gp_camera_folder_list_files(theCamera, "/", &list) != GP_OK)
        throw QString("Could not retrieve picture list");

    /* Iterate over whole list */
    count = gp_list_count(&list);
    for (int i=0; i < count; i++)  {
        gp_list_get_name (&list, i, &name);
        
        /* Download thumb & insert in icon view */
        QIconViewItem* icon = new QIconViewItem(iconView,0,name,
                                  downloadThumb(name,"/"));
        icon->setDragEnabled(false);
        icon->setRenameEnabled(false);
    }
}


/* Downloads the thumb with the given name */
QPixmap GPInterface::downloadThumb(const char* name, const char* folder)
{
    QPixmap p;
    CameraFile *f;
    const char* data;
    long int size;
    
    /* Initialize file */
    gp_file_new(&f); 

    /* Try downloading thumb */
    if (gp_camera_file_get(theCamera, folder, name,
                           GP_FILE_TYPE_PREVIEW, f) != GP_OK) 
        throw QString("Couldn't get thumb ") + QString(name) +
              QString(" in folder ") + QString(folder);
    
    /* Construct Pixmap */
    gp_file_get_data_and_size(f,&data,&size);
    p.loadFromData((const uchar*) data, (uint) size);
    
    /* Release file */
    gp_file_free(f);
    
    return p;
}


/*
 * Interface to gPhoto2.
 * gPhoto2 sends messages to the GUI using these procedures.
 */
int GPInterface::frontend_status(Camera * /*camera*/, char * /*message*/)
{
  printf("Frontend is calling status\n");
   return 0;
}

int GPInterface::frontend_progress(Camera*, CameraFile*, float percentage)
{
    GPMessenger::instance()->emit progressChanged((int) percentage);
    return 0;
}

int GPInterface::frontend_message(Camera * /*camera*/, char * /*message*/)
{
  printf("Frontend is calling message\n");
  
  return 0;
}

int GPInterface::frontend_confirm(Camera * /*camera*/, char * /*message*/)
{
  printf("Frontend is calling confirm\n");
   return 0;
}

int GPInterface::frontend_prompt (Camera * /*camera*/, CameraWidget * /*window*/)
{
  printf("Frontend is calling prompt\n");
  return 0;
}

/* Static instances */
bool GPInterface::cameraInitialized = false;
Camera* GPInterface::theCamera = 0;
