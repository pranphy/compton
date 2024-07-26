#ifndef __COMPTONHEPEVTINTERFACE_HH
#define __COMPTONHEPEVTINTERFACE_HH

#include "G4VPrimaryGenerator.hh"

class G4Event;
class comptonHEPEvtMessenger;

class comptonHEPEvtInterface : public G4VPrimaryGenerator {
protected:
  G4int fVerbose;
  G4String fFilename;

  comptonHEPEvtMessenger* fMessenger;

  static G4VPrimaryGenerator* fHEPEvtInterface;

public:
  comptonHEPEvtInterface();
  virtual ~comptonHEPEvtInterface();

  // set/get methods
  void SetFileName(G4String name);
  G4String GetFileName() const;

  void SetVerboseLevel(G4int i);
  G4int GetVerboseLevel() const;

  // methods...
  void Initialize();

  virtual void GeneratePrimaryVertex(G4Event* anEvent);
};

// ====================================================================
// inline functions
// ====================================================================

inline void comptonHEPEvtInterface::SetFileName(G4String name)
{
  fFilename = name;
}

inline G4String comptonHEPEvtInterface::GetFileName() const
{
  return fFilename;
}

inline void comptonHEPEvtInterface::SetVerboseLevel(G4int i)
{
  fVerbose = i;
}

inline G4int comptonHEPEvtInterface::GetVerboseLevel() const
{
  return fVerbose;
}

#endif
