#include "GPMessenger.h"
#include "GPMessenger.moc"

GPMessenger* GPMessenger::instance()
{ 
    if (!inst) 
        inst = new GPMessenger(); 
    return inst; 
}
                   
GPMessenger* GPMessenger::inst = 0;

