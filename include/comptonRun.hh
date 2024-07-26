#ifndef __COMPTONRUN_HH
#define __COMPTONRUN_HH

/*!
 * All the information on the run
 * The data object will get put into the output
 * stream

   This is implemented in the soliton model
 */

#include "G4Run.hh"

class comptonRunData;

class comptonRun: public G4Run {

  public:
    comptonRun(): G4Run() { };
    virtual ~comptonRun() { };

  // Static run data access
  private:
    static comptonRunData* fRunData;
  public:
    static comptonRunData* GetRunData();
};

#endif //__COMPTONRUN_HH
