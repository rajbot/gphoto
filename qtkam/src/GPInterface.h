/**
 * The interface with gphoto.
 * All communication between gPhoto and QtKam is done through
 * this class.
 */

#ifndef GPINTERFACE_H
#define GPINTERFACE_H

#include <gphoto2.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qiconview.h>
#include <qpixmap.h>
#include <qrect.h>


/** 
 * The GPInterface class definition.
 * All methods are static.
 */
class GPInterface 
{
 public:
    static void initialize(); 
    static void shutdown();
    
    static void initCamera();
    static void setCamera(const QString& camera);
    static void setPort(const QString& port);
    static void setSpeed(const QString& speed);
    static void setFolder(const QString& folder);
    static void setTempFolder(const QString& folder);
    static void setGeometry(const QRect& geometry);

    static QString getCamera();
    static QString getPort();
    static QString getSpeed();
    static QString getFolder();
    static QString getTempFolder();
    static QRect getGeometry();
    static QStringList getSupportedCameras(); 
    static QStringList getSupportedPorts(const QString& camera);
    static QStringList getSupportedSpeeds(const QString& camera);
    static void downloadThumbs(QIconView* iconView);

private:
    static QPixmap downloadThumb(const char* name, const char* folder);
    static void initTempFolder();
    static void deleteTempFolder();
    
    static int frontend_status(Camera*, char*);
    static int frontend_progress(Camera*, CameraFile*, float);
    static int frontend_message(Camera*, char*);
    static int frontend_confirm(Camera*, char*);
    static int frontend_prompt (Camera*, CameraWidget*);

    static bool cameraInitialized;
    static Camera* theCamera;
};
    
#endif
