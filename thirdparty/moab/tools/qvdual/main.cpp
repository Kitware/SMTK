#include <qapplication.h>
#include "uiQVDual.h"
#include "MBInterface.hpp"

MBInterface *gMB = NULL;

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    uiQVDual w;

    int cur_arg = 1;
    if (cur_arg < argc && !strcmp(argv[cur_arg], "-d")) {
      w.computeDual = true;
      cur_arg++;
    }
    
    w.show();
    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    if (cur_arg < argc) w.fileOpen(QString(argv[cur_arg]));
    
    return a.exec();
}
