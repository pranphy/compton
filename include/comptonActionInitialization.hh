/*
 * comptonUserActionInitialization.hh
 *
 *  Created on: May 1, 2017
 *      Author: wdconinc
 */

#ifndef __REMOLLACTIONINITIALIZATION_HH
#define __REMOLLACTIONINITIALIZATION_HH

#include "G4VUserActionInitialization.hh"

class comptonActionInitialization : public G4VUserActionInitialization {
  public:
    comptonActionInitialization(): G4VUserActionInitialization() { };
    virtual ~comptonActionInitialization() { };

    virtual void Build() const;
    virtual void BuildForMaster() const;
};

#endif // __REMOLLACTIONINITIALIZATION_HH
