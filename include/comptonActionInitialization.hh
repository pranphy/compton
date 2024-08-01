/*
 * comptonUserActionInitialization.hh
 *
 *  Created on: May 1, 2017
 *      Author: wdconinc
 */

#ifndef __COMPTONACTIONINITIALIZATION_HH
#define __COMPTONACTIONINITIALIZATION_HH

#include "G4VUserActionInitialization.hh"

class comptonActionInitialization : public G4VUserActionInitialization {
  public:
    comptonActionInitialization(): G4VUserActionInitialization() { };
    virtual ~comptonActionInitialization() { };

    virtual void Build() const;
    virtual void BuildForMaster() const;
};

#endif // __COMPTONACTIONINITIALIZATION_HH
