
#include "CGBL.h"
#include "Util.h"
#include "Tool.h"


int main( int argc, char *argv[] )
{
    setLogFileName( "NIScaler.log" );

    if( !GBL.SetCmdLine( argc, argv ) ) {
        Log();
        return 42;
    }

//double qq=getTime();

    Tool    tool;
    tool.entrypoint();

//Log()<<"tot secs "<<getTime()-qq;

    Log();
    return 0;
}


