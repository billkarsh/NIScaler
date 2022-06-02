#ifndef TOOL_H
#define TOOL_H

#include "KVParams.h"

#include <QSettings>

#include <vector>

/* ---------------------------------------------------------------- */
/* Types ---------------------------------------------------------- */
/* ---------------------------------------------------------------- */

struct Coeff {
// Table of cal coeffs for given dev_product_voltage...
// Addressed by physical NI channel.
    int ncof;
    std::vector<std::vector<double> >   V;
    Coeff() : ncof(0)   {}
    void get( QSettings &S, const QString &grpdev );
};

struct Plan {
// At each timepoint...
// Which {Coeff table, physical channel} to apply
    double              V2I;    // volts -> i16
    int                 nC,     // words/timepoint
                        nai;    // vector size
    std::vector<bool>   ic2K1;  // true if K1
    std::vector<uint>   ic2ai;  // physical channel
    void make( const KVParams &kvp );
    void apply( qint16 *d, int ntpts, const Coeff &K1, const Coeff &K2 ) const;
};

class Tool
{
public:
    virtual ~Tool() {}

    void entrypoint();

private:
    bool createCal();
    void apply();
    bool okInput();
    bool enumSrc( QStringList &sl );
    bool do1_ok_meta( KVParams &kvp, const QString &s );
    bool do1_ok_coef(
        Coeff           &K1,
        Coeff           &K2,
        QSettings       &S,
        const KVParams  &kvp,
        const QString   &s );
    bool do1_update_meta( const QString &s, KVParams &kvp );
    void do1_scale(
        const QString   &s,
        const Plan      &P,
        const Coeff     &K1,
        const Coeff     &K2 );
    QString meta2bin( const QString &meta );
};

#endif  // TOOL_H


