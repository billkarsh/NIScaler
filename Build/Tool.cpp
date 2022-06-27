
#include "Tool.h"
#include "CGBL.h"
#include "Util.h"
#include "Subset.h"

#ifdef HAVE_NIDAQmx
#include "NIDAQmx.h"
#else
#pragma message("*** Message to self: Building simulated NI-DAQ version ***")
#endif

#include <QDirIterator>


/* ---------------------------------------------------------------- */
/* Statics -------------------------------------------------------- */
/* ---------------------------------------------------------------- */

// ------
// Macros
// ------

#define DAQmxErrChk(functionCall)                           \
    do {                                                    \
    if( DAQmxFailed(dmxErrNum = (functionCall)) )           \
        {dmxFnName = STR(functionCall); goto Error_Out;}    \
    } while( 0 )

// ----
// Data
// ----

#ifdef HAVE_NIDAQmx
static QVector<char>    dmxErrMsg;
static const char       *dmxFnName;
static int32            dmxErrNum;
#endif

// -------
// Methods
// -------

/* ---------------------------------------------------------------- */
/* clearDmxErrors ------------------------------------------------- */
/* ---------------------------------------------------------------- */

#ifdef HAVE_NIDAQmx
static void clearDmxErrors()
{
    dmxErrMsg.clear();
    dmxFnName   = "";
    dmxErrNum   = 0;
}
#endif

/* ---------------------------------------------------------------- */
/* lastDAQErrMsg -------------------------------------------------- */
/* ---------------------------------------------------------------- */

// Capture latest dmxErrNum as a descriptive C-string.
// Call as soon as possible after offending operation.
//
#ifdef HAVE_NIDAQmx
static void lastDAQErrMsg()
{
    const int msgbytes = 2048;
    dmxErrMsg.resize( msgbytes );
    dmxErrMsg[0] = 0;
    DAQmxGetExtendedErrorInfo( &dmxErrMsg[0], msgbytes );
}
#endif

/* ---------------------------------------------------------------- */
/* destroyTask ---------------------------------------------------- */
/* ---------------------------------------------------------------- */

#ifdef HAVE_NIDAQmx
static void destroyTask( TaskHandle &taskHandle )
{
    if( taskHandle ) {
        DAQmxStopTask( taskHandle );
        DAQmxClearTask( taskHandle );
        taskHandle = 0;
    }
}
#endif

/* ---------------------------------------------------------------- */
/* isHardware ----------------------------------------------------- */
/* ---------------------------------------------------------------- */

// Fill niDevNames.
//
// Return true if non-empty.
//
#ifdef HAVE_NIDAQmx
bool isHardware( QStringList &niDevNames )
{
    char    data[2048] = {0};

    niDevNames.clear();

    if( DAQmxFailed( DAQmxGetSysDevNames( data, sizeof(data) ) ) )
        return false;

    niDevNames = QString(data).split(
                                QRegExp("\\s*,\\s*"),
                                QString::SkipEmptyParts );

    return !niDevNames.isEmpty();
}
#endif

/* ---------------------------------------------------------------- */
/* getPhysChans --------------------------------------------------- */
/* ---------------------------------------------------------------- */

#ifdef HAVE_NIDAQmx
typedef int32 (__CFUNC *QueryFunc_t)( const char [], char*, uInt32 );

static QStringList getPhysChans(
    const QString   &dev,
    QueryFunc_t     queryFunc,
    const QString   &fn = QString() )
{
    QString         funcName = fn;
    QVector<char>   buf( 65536 );

    if( !funcName.length() )
        funcName = "??";

    buf[0] = 0;

    clearDmxErrors();

    DAQmxErrChk( queryFunc( STR2CHR( dev ), &buf[0], buf.size() ) );

    // "\\s*,\\s*" encodes <optional wh spc><comma><optional wh spc>
    return QString( &buf[0] )
            .split( QRegExp("\\s*,\\s*"), QString::SkipEmptyParts );

Error_Out:
    if( DAQmxFailed( dmxErrNum ) ) {

        lastDAQErrMsg();

        Log()
            << "DAQmx Error: Fun=<"
            << funcName
            << "> Bufsz=("
            << buf.size()
            << ") Err=<"
            << dmxErrNum
            << "> '" << &dmxErrMsg[0] << "'.";
    }

    return QStringList();
}
#endif

/* ---------------------------------------------------------------- */
/* getAIChans ----------------------------------------------------- */
/* ---------------------------------------------------------------- */

// Gets entries of type "Dev6/ai5"
//
#ifdef HAVE_NIDAQmx
static QStringList getAIChans( const QString &dev )
{
    return getPhysChans( dev,
            DAQmxGetDevAIPhysicalChans,
            "DAQmxGetDevAIPhysicalChans" );
}
#endif

/* ---------------------------------------------------------------- */
/* probeAllAIRanges ----------------------------------------------- */
/* ---------------------------------------------------------------- */

#ifdef HAVE_NIDAQmx
static void getAIRangeMaxima( QVector<double> &rmax, const QString &dev )
{
    rmax.clear();

    float64 dArr[512];
    memset( dArr, 0, sizeof(dArr) );

    if( !DAQmxFailed( DAQmxGetDevAIVoltageRngs(
            STR2CHR( dev ),
            dArr,
            512 ) ) ) {

        for( int i = 0; i < 512; i +=2 ) {

            double  vmax = dArr[i+1];

            if( vmax == dArr[i] )
                break;

            rmax.push_back( vmax );
        }
    }
}
#endif

/* ---------------------------------------------------------------- */
/* getProductName ------------------------------------------------- */
/* ---------------------------------------------------------------- */

#ifdef HAVE_NIDAQmx
static QString getProductName( const QString &dev )
{
    QVector<char>   buf( 65536 );
    strcpy( &buf[0], "Unknown" );

    if( DAQmxFailed(
        DAQmxGetDevProductType(
        STR2CHR( dev ), &buf[0], buf.size() ) ) ) {

        Log()
            << "DAQmx Error: Failed query of product name for dev "
            << dev << ".";
    }

    return &buf[0];
}
#endif

/* ---------------------------------------------------------------- */
/* createTask ----------------------------------------------------- */
/* ---------------------------------------------------------------- */

#ifdef HAVE_NIDAQmx
TaskHandle createTask( const QString &dev, double vmax, int nChan )
{
    QString     chnRng = QString("/%1/ai0:%2").arg( dev ).arg( nChan-1 );
    TaskHandle  task;
    int         ret;

    ret = DAQmxCreateTask( "", &task );

    if( DAQmxFailed( ret ) )
        return 0;

    ret = DAQmxCreateAIVoltageChan(
            task,
            STR2CHR( chnRng ),
            "",
            -1,     // default termination
            -vmax,
            vmax,
            DAQmx_Val_Volts,
            NULL );

    if( DAQmxFailed( ret ) )
        goto fail;

    ret = DAQmxCfgSampClkTiming( task,
            "", 100, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 1000 );

    if( DAQmxFailed( ret ) )
        goto fail;

    ret = DAQmxTaskControl( task, DAQmx_Val_Task_Commit );

    if( DAQmxFailed( ret ) && ret != DAQmxErrorExtSampClkSrcNotSpecified )
        goto fail;

    return task;

fail:
    destroyTask( task );
    return 0;
}
#endif

/* ---------------------------------------------------------------- */
/* Coeff ---------------------------------------------------------- */
/* ---------------------------------------------------------------- */

void Coeff::get( QSettings &S, const QString &grpdev )
{
    S.beginGroup( grpdev );

    int nai = S.value( "nai", 0 ).toInt();

    V.resize( nai );

    for( int ic = 0; ic < nai; ++ic ) {

        std::vector<double> &C = V[ic];

        QStringList sl = S.value( QString("ai%1").arg( ic ) ).toStringList();
        ncof = sl.size();

        for( int k = 0; k < ncof; ++k )
            C.push_back( sl.at( k ).toDouble() );
    }

    S.endGroup();
}

/* ---------------------------------------------------------------- */
/* Plan ----------------------------------------------------------- */
/* ---------------------------------------------------------------- */

void Plan::make( const KVParams &kvp )
{
    std::vector<bool>   vK1;  // true if K1
    std::vector<uint>   vai;  // physical channel
    QVector<uint>       vi;
    int                 kmux = kvp["niMuxFactor"].toInt();
    int                 nv;
    bool                dual = kvp["niDualDevMode"].toBool();

    V2I = 32768 / kvp["niAiRangeMax"].toDouble();
    nC  = kvp["nSavedChans"].toInt();

// First fill {vK1, vai} with acquired NI channels

// MN

    QString chnstr = kvp["niMNChans1"].toString();

    if( !chnstr.isEmpty() ) {

        Subset::rngStr2Vec( vi, chnstr );

        for( int ic = 0, nc = vi.size(); ic < nc; ++ic ) {

            uint    ai = vi[ic];

            for( int j = 0; j < kmux; ++j ) {
                vK1.push_back( true );
                vai.push_back( ai );
            }
        }
    }

    if( dual ) {

        chnstr = kvp["niMNChans2"].toString();

        if( !chnstr.isEmpty() ) {

            Subset::rngStr2Vec( vi, chnstr );

            for( int ic = 0, nc = vi.size(); ic < nc; ++ic ) {

                uint    ai = vi[ic];

                for( int j = 0; j < kmux; ++j ) {
                    vK1.push_back( false );
                    vai.push_back( ai );
                }
            }
        }
    }

// MA

    chnstr = kvp["niMAChans1"].toString();

    if( !chnstr.isEmpty() ) {

        Subset::rngStr2Vec( vi, chnstr );

        for( int ic = 0, nc = vi.size(); ic < nc; ++ic ) {

            uint    ai = vi[ic];

            for( int j = 0; j < kmux; ++j ) {
                vK1.push_back( true );
                vai.push_back( ai );
            }
        }
    }

    if( dual ) {

        chnstr = kvp["niMAChans2"].toString();

        if( !chnstr.isEmpty() ) {

            Subset::rngStr2Vec( vi, chnstr );

            for( int ic = 0, nc = vi.size(); ic < nc; ++ic ) {

                uint    ai = vi[ic];

                for( int j = 0; j < kmux; ++j ) {
                    vK1.push_back( false );
                    vai.push_back( ai );
                }
            }
        }
    }

// XA

    chnstr = kvp["niXAChans1"].toString();

    if( !chnstr.isEmpty() ) {

        Subset::rngStr2Vec( vi, chnstr );

        for( int ic = 0, nc = vi.size(); ic < nc; ++ic ) {

            vK1.push_back( true );
            vai.push_back( vi[ic] );
        }
    }

    if( dual ) {

        chnstr = kvp["niXAChans2"].toString();

        if( !chnstr.isEmpty() ) {

            Subset::rngStr2Vec( vi, chnstr );

            for( int ic = 0, nc = vi.size(); ic < nc; ++ic ) {

                vK1.push_back( false );
                vai.push_back( vi[ic] );
            }
        }
    }

// Next copy only saved channels to plan

    QBitArray   b;

    chnstr  = kvp["snsSaveChanSubset"].toString();
    nv      = vai.size();

    if( Subset::isAllChansStr( chnstr ) )
        Subset::defaultBits( b, nv );
    else
        Subset::rngStr2Bits( b, chnstr );

    for( int ib = 0; ib < nv; ++ib ) {

        if( b.testBit( ib ) ) {
            ic2K1.push_back( vK1[ib] );
            ic2ai.push_back( vai[ib] );
        }
    }

    nai = ic2ai.size();
}


void Plan::apply( qint16 *d, int ntpts, const Coeff &K1, const Coeff &K2 ) const
{
    for( int it = 0; it < ntpts; ++it, d += nC ) {

        for( int ic = 0; ic < nai; ++ic ) {

            double          V = 0.0;
            const double    *C;
            int             ncof;

            if( ic2K1[ic] ) {
                C       = &K1.V[ic2ai[ic]][0];
                ncof    = K1.ncof;
            }
            else {
                C       = &K2.V[ic2ai[ic]][0];
                ncof    = K2.ncof;
            }

            for( int k = ncof - 1; k > 0; --k ) {
                V += C[k];
                V *= d[ic];
            }

            d[ic] = qBound( SHRT_MIN, int(V2I * (V + C[0])), SHRT_MAX );
        }
    }
}

/* ---------------------------------------------------------------- */
/* Tool ----------------------------------------------------------- */
/* ---------------------------------------------------------------- */

void Tool::entrypoint()
{
    if( GBL.create && !createCal() )
        return;

    if( !GBL.apply )
        return;

    apply();
}


// Return true if no errors.
//
#ifdef HAVE_NIDAQmx
bool Tool::createCal()
{
    QStringList  niDevNames;

    if( !isHardware( niDevNames ) ) {
        Log() << "No NI hardware detected.";
        return true;
    }

    bool    isAIDev = false;

    foreach( const QString &D, niDevNames ) {

        QVector<double> rmax;
        getAIRangeMaxima( rmax, D );

        if( rmax.isEmpty() )
            continue;

        QStringList aiChans = getAIChans( D );
        int         nC      = aiChans.size();

        if( !nC )
            continue;

        isAIDev = true;

        foreach( double vmax, rmax ) {

            TaskHandle  T = createTask( D, vmax, nC );

            if( !T ) {
                Log()
                    << "DAQmx Error: Failed creating task for dev "
                    << D << ".";
                continue;
            }

            QSettings   S( GBL.calFile(), QSettings::IniFormat );
            QString     grpdev = QString("%1_%2_V%3")
                                    .arg( D )
                                    .arg( getProductName( D ) )
                                    .arg( vmax );
            S.remove( grpdev );
            S.beginGroup( grpdev );
            S.setValue( "nai", nC );

            int ic = -1;

            foreach( const QString &chan, aiChans ) {

                float64     f64[4]; // poly max order = 3
                QStringList sl;
                int         ncof;

                ++ic;

                // Get coeffs

                if( DAQmxFailed(
                    DAQmxGetAIDevScalingCoeff(
                    T, STR2CHR(chan), f64, sizeof(f64)/sizeof(float64) ) ) ) {

                    Log()
                        << "DAQmx Error: Failed query of cal coeffs for dev "
                        << D << " chan " << ic << ".";
                    S.remove( grpdev );
                    goto next_dev;
                }

                // Coeff count

                ncof = DAQmxGetAIDevScalingCoeff( T, STR2CHR(chan), 0, 0 );

                // Store

                for( int i = 0; i < ncof; ++i )
                    sl += QString("%1").arg( f64[i] );

                S.setValue( QString("ai%1").arg( ic ), sl );
            }

next_dev:
            destroyTask( T );
        }
    }

    if( !isAIDev )
        Log() << "No NI analog input devices detected.";

    return true;
}
#else
bool Tool::createCal()
{
    Log() << "Error: Option <-create_cal> NOT SUPPORTED.";
    return false;
}
#endif


void Tool::apply()
{
    if( !okInput() )
        return;

    QStringList sl;

    if( !enumSrc( sl ) )
        return;

    GBL.src_dir += "/";
    GBL.dst_dir += "/";

    QSettings   S( GBL.calFile(), QSettings::IniFormat );

    foreach( const QString &s, sl ) {

        Coeff       K1, K2;
        KVParams    kvp;

        if( do1_ok_meta( kvp, s ) &&
            do1_ok_coef( K1, K2, S, kvp, s ) &&
            do1_update_meta( s, kvp ) ) {

            Plan    P;
            P.make( kvp );

            do1_scale( s, P, K1, K2 );
        }
    }
}


bool Tool::okInput()
{
    QFileInfo   fi;

    fi.setFile( GBL.src_dir );

    if( !fi.exists() ) {
        Log() << QString("Error: Dir not found <%1>.").arg( GBL.src_dir );
        return false;
    }

    fi.setFile( GBL.dst_dir );

    if( !fi.exists() ) {
        Log() << QString("Error: Dir not found <%1>.").arg( GBL.dst_dir );
        return false;
    }

    fi.setFile( GBL.calFile() );

    if( !fi.exists() ) {
        Log() << QString("Error: File not found <%1>.").arg( GBL.calFile() );
        return false;
    }

    return true;
}


bool Tool::enumSrc( QStringList &sl )
{
    QDirIterator    it( GBL.src_dir );

    while( it.hasNext() ) {

        it.next();

        QFileInfo   fi      = it.fileInfo();
        QString     entry   = fi.fileName();

        if( fi.isFile() && entry.endsWith( ".meta" ) ) {

            sl.append( entry );
            Log() << "Found " << entry;
        }
    }

    return sl.size() != 0;
}


bool Tool::do1_ok_meta( KVParams &kvp, const QString &s )
{
// Bin exists

    QString sbin = meta2bin( s );

    if( !QFileInfo( GBL.src_dir + sbin ).exists() ) {
        Log() << QString("Binary file not found '%1'.").arg( sbin );
        return false;
    }

// Open

    if( !kvp.fromMetaFile( GBL.src_dir + s ) ) {
        Log() << QString("Meta file is corrupt '%1'.").arg( s );
        return false;
    }

// Qualify

    if( kvp["appVersion"].toString() >= "20220101" ) {
        Log() << QString("Skipping (SpikeGLX version [%1] too new) '%2'.")
                    .arg( kvp["appVersion"].toString() ).arg( s );
        return false;
    }

    if( kvp.contains( "NIScaler" ) ) {
        Log() << QString("Skipping (Already scaled [%1]) '%2'.")
                    .arg( kvp["NIScaler"].toString() ).arg( s );
        return false;
    }

    return true;
}


bool Tool::do1_ok_coef(
    Coeff           &K1,
    Coeff           &K2,
    QSettings       &S,
    const KVParams  &kvp,
    const QString   &s )
{
    QString grpdev =
        QString("%1_%2_V%3")
        .arg( GBL.dev1.isEmpty() ? kvp["niDev1"].toString() : GBL.dev1 )
        .arg( kvp["niDev1ProductName"].toString() )
        .arg( kvp["niAiRangeMax"].toDouble() );

    if( !S.contains( grpdev + "/nai" ) ) {
        Log() << QString("Skipping (Missing dev1 cal table [%1]) '%2'.")
                    .arg( grpdev ).arg( s );
        return false;
    }

    K1.get( S, grpdev );

    if( kvp.contains( "niDualDevMode" ) ) {

        grpdev =
            QString("%1_%2_V%3")
            .arg( GBL.dev2.isEmpty() ? kvp["niDev2"].toString() : GBL.dev2 )
            .arg( kvp["niDev2ProductName"].toString() )
            .arg( kvp["niAiRangeMax"].toDouble() );

        if( !S.contains( grpdev + "/nai" ) ) {
            Log() << QString("Skipping (Missing dev2 cal table [%1]) '%2'.")
                        .arg( grpdev ).arg( s );
            return false;
        }

        K2.get( S, grpdev );
    }

    return true;
}


bool Tool::do1_update_meta( const QString &s, KVParams &kvp )
{
// Date-time stamp

    kvp["NIScaler"] =
    dateTime2Str( QDateTime(QDateTime::currentDateTime()), Qt::ISODate );

// Write

    if( !kvp.toMetaFile( GBL.dst_dir + s ) ) {
        Log() << QString("Error writing metafile '%1'.").arg( s );
        return false;
    }

    return true;
}


void Tool::do1_scale(
    const QString   &s,
    const Plan      &P,
    const Coeff     &K1,
    const Coeff     &K2 )
{
#define BUFBYTES    128*1024

    QString sbin = meta2bin( s );
    QFile   fa( GBL.src_dir + sbin );
    QFile   fb( GBL.dst_dir + sbin );

    fa.open( QIODevice::ReadOnly );
    fb.open( QIODevice::WriteOnly );

    std::vector<char>   buf( BUFBYTES );

    quint64 asmp    = fa.size() / (2 * P.nC),
            bufsmp  = BUFBYTES / (2 * P.nC);

    while( asmp ) {

        int smp = qMin( bufsmp, asmp );

        fa.read( &buf[0], 2 * P.nC * smp );

        P.apply( (qint16*)&buf[0], smp, K1, K2 );

        fb.write( &buf[0], 2 * P.nC * smp );

        asmp -= smp;
    }
}


QString Tool::meta2bin( const QString &meta )
{
    QRegExp re("meta$");
    re.setCaseSensitivity( Qt::CaseInsensitive );

    return QString(meta).replace( re, "bin" );
}


