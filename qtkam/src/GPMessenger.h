/**
 * The messenger. 
 * This object sends signals from gphoto2.
 * This is a Singleton class.
 */

#ifndef GPMESSENGER_H
#define GPMESSENGER_H

#include <qobject.h>

class GPInterface;

class GPMessenger : public QObject
{
    Q_OBJECT
    
    friend class GPInterface;

    public:
        static GPMessenger* instance();
    
    signals:
        void progressChanged(int p);    
         
    private:
        GPMessenger() : QObject() { };
        static GPMessenger *inst;
};

#endif
