#ifndef __REMOLLGENLUND_HH 
#define __REMOLLGENLUND_HH 
/*!
 * event generator based on LUND files
 *
 * Rakitha Beminiwattha
 * Fri Apr 15 12:51:25 EDT 2016
 *
 */

// compton includes
#include "comptonVEventGen.hh"

// System includes
#include <map>
#include <fstream>

class comptonRunData;

class comptonGenLUND : public comptonVEventGen {

  public:
    // constructor
    comptonGenLUND();

    // virtual destructor
    virtual ~comptonGenLUND();

    void SetLUNDFile(G4String& f);

  private:
    void SamplePhysics(comptonVertex *, comptonEvent *);

    comptonRunData *fRunData;

    std::ifstream LUNDfile;
    size_t LUNDfile_linecount;

    G4bool bLUND;

    std::map<int,G4String> pidname;
};

#endif//__REMOLLGENLUND_HH 
