/* comptonHEPEvtInterface

This class reads files written as a /HEPEVT/ common block by the
following code (examples/extended/runAndEvent/RE05).


***********************************************************
      SUBROUTINE HEP2G4
*
* Output /HEPEVT/ event structure to G4HEPEvtInterface
*
* M.Asai (asai@kekvax.kek.jp)  --  24/09/96
*
***********************************************************
      PARAMETER (NMXHEP=4000)
      COMMON/HEPEVT/NEVHEP,NHEP,ISTHEP(NMXHEP),IDHEP(NMXHEP),
     >JMOHEP(2,NMXHEP),JDAHEP(2,NMXHEP),PHEP(5,NMXHEP),VHEP(4,NMXHEP)
      DOUBLE PRECISION PHEP,VHEP
*
      WRITE(6,*) NHEP
      DO IHEP=1,NHEP
       WRITE(6,10)
     >  ISTHEP(IHEP),IDHEP(IHEP),JDAHEP(1,IHEP),JDAHEP(2,IHEP),
     >  PHEP(1,IHEP),PHEP(2,IHEP),PHEP(3,IHEP),PHEP(5,IHEP)
10    FORMAT(I4,I10,I5,I5,4(1X,D15.8))
      ENDDO
*
      RETURN
      END

*/

#include "comptonHEPEvtInterface.hh"
#include "comptonHEPEvtMessenger.hh"

#include "G4Event.hh"
#include "G4HEPEvtInterface.hh"
#include "G4AutoLock.hh"

namespace {
 G4Mutex comptonHEPEvtInterfaceMutex = G4MUTEX_INITIALIZER;
}

G4VPrimaryGenerator* comptonHEPEvtInterface::fHEPEvtInterface = 0;

comptonHEPEvtInterface::comptonHEPEvtInterface()
: fVerbose(0), fFilename("hepevt.dat")
{
  fMessenger = new comptonHEPEvtMessenger(this);
}

comptonHEPEvtInterface::~comptonHEPEvtInterface()
{
  G4AutoLock lock(&comptonHEPEvtInterfaceMutex);
  if(fHEPEvtInterface != nullptr) { delete fHEPEvtInterface; fHEPEvtInterface = 0; }
  delete fMessenger;
}

void comptonHEPEvtInterface::Initialize()
{
  G4AutoLock lock(&comptonHEPEvtInterfaceMutex);
  if (fHEPEvtInterface != nullptr) { delete fHEPEvtInterface; fHEPEvtInterface = 0; }
  fHEPEvtInterface = new G4HEPEvtInterface(fFilename,fVerbose);
}

void comptonHEPEvtInterface::GeneratePrimaryVertex(G4Event* anEvent)
{
  G4AutoLock lock(&comptonHEPEvtInterfaceMutex);
  if (fHEPEvtInterface == nullptr) { Initialize(); }
  fHEPEvtInterface->GeneratePrimaryVertex(anEvent);
}
