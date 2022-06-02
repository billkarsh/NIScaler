
#include "CGBL.h"
#include "Cmdline.h"
#include "Util.h"


/* --------------------------------------------------------------- */
/* Globals ------------------------------------------------------- */
/* --------------------------------------------------------------- */

CGBL    GBL;

/* --------------------------------------------------------------- */
/* PrintUsage  --------------------------------------------------- */
/* --------------------------------------------------------------- */

static void PrintUsage()
{
    Log();
    Log() << "*** ERROR: MISSING CRITICAL PARAMETERS ***\n";
    Log() << "------------------------";
    Log() << "Purpose:";
    Log() << "+ Rescale SpikeGLX NI data acquired before version 20220101:";
    Log() << "+ Create voltage calibration files for NI devices.";
    Log() << "+ Apply corrections to existing SpikeGLX NI bin/meta files.";
    Log() << "Run messages are appended to NIScaler.log in the current working directory.\n";
    Log() << "Output:";
    Log() << "+ Calibration files are placed into 'cal_dir'.";
    Log() << "+ Corrected NI data files are placed (with same name) into 'dst_dir' folder.\n";
    Log() << "Usage:";
    Log() << ">NIScaler < parameters >\n";
    Log() << "Parameters:";
    Log() << "-create_cal     ;scan NI devices and create calibration files";
    Log() << "-apply          ;use calibration files to correct SpikeGLX NI data";
    Log() << "-cal_dir=path   ;where to put/get calibration files";
    Log() << "-src_dir=path   ;if applying, directory with nidq.bin/meta files to fix";
    Log() << "-dst_dir=path   ;if applying, where to put fixed nidq.bin/meta files";
    Log() << "-dev1=new_name  ;optional new name of dev1 if moved or renamed since run";
    Log() << "-dev2=new_name  ;optional new name of dev2 if moved or renamed since run";
    Log() << "------------------------\n";
}

/* ---------------------------------------------------------------- */
/* CGBL ----------------------------------------------------------- */
/* ---------------------------------------------------------------- */

bool CGBL::SetCmdLine( int argc, char* argv[] )
{
// Parse args

    const char  *sarg = 0;

    for( int i = 1; i < argc; ++i ) {

        if( GetArgStr( sarg, "-cal_dir=", argv[i] ) )
            cal_dir = trim_adjust_slashes( sarg );
        else if( GetArgStr( sarg, "-src_dir=", argv[i] ) )
            src_dir = trim_adjust_slashes( sarg );
        else if( GetArgStr( sarg, "-dst_dir=", argv[i] ) )
            dst_dir = trim_adjust_slashes( sarg );
        else if( GetArgStr( sarg, "-dev1=", argv[i] ) )
            dev1 = sarg;
        else if( GetArgStr( sarg, "-dev2=", argv[i] ) )
            dev2 = sarg;
        else if( IsArg( "-create_cal", argv[i] ) )
            create = true;
        else if( IsArg( "-apply", argv[i] ) )
            apply = true;
        else {
            Log() <<
            QString("Unknown option or wrong param count for option '%1'.")
            .arg( argv[i] );
            return false;
        }
    }

// Check args

    if( !create && !apply ) {
        Log() << "Error: Missing action indicator {-create_cal, -apply}.";
        goto error;
    }

    if( cal_dir.isEmpty() ) {
        Log() << "Error: Missing -cal_dir.";
        goto error;
    }

    if( apply ) {

        if( src_dir.isEmpty() ) {
            Log() << "Error: Missing -src_dir.";
            goto error;
        }

        if( dst_dir.isEmpty() ) {
            Log() << "Error: Missing -dst_dir.";
error:
            PrintUsage();
            return false;
        }
    }

// Echo

    sCmd = "NIScaler";

    if( create )
        sCmd += " -create_cal";

    if( apply )
        sCmd += " -apply";

    sCmd += " -cal_dir=" + cal_dir;

    if( apply ) {
        sCmd += " -src_dir=" + src_dir;
        sCmd += " -dst_dir=" + dst_dir;

        if( !dev1.isEmpty() )
            sCmd += " -dev1=" + dev1;

        if( !dev2.isEmpty() )
            sCmd += " -dev2=" + dev2;
    }

    Log() << QString("Cmdline: %1").arg( sCmd );

    return true;
}


QString CGBL::calFile()
{
    return QString("%1/niscaler_cal.ini").arg( cal_dir );
}

/* --------------------------------------------------------------- */
/* Private ------------------------------------------------------- */
/* --------------------------------------------------------------- */

QString CGBL::trim_adjust_slashes( const QString &dir )
{
    QString s = dir.trimmed();

    s.replace( "\\", "/" );
    return s.remove( QRegExp("/+$") );
}


