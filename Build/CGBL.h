#ifndef CGBL_H
#define CGBL_H

#include <QString>

/* ---------------------------------------------------------------- */
/* Types ---------------------------------------------------------- */
/* ---------------------------------------------------------------- */

class CGBL
{
public:
    QString     sCmd,
                cal_dir,
                src_dir,
                dst_dir,
                dev1,
                dev2;
    bool        create,
                apply;

public:
    CGBL() : create(false), apply(false)    {}

    bool SetCmdLine( int argc, char* argv[] );

    QString calFile();

private:
    QString trim_adjust_slashes( const QString &dir );
};

/* --------------------------------------------------------------- */
/* Globals ------------------------------------------------------- */
/* --------------------------------------------------------------- */

extern CGBL GBL;

#endif  // CGBL_H


