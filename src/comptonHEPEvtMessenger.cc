#include "comptonHEPEvtMessenger.hh"
#include "comptonHEPEvtInterface.hh"

#include "G4UIdirectory.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"

comptonHEPEvtMessenger::comptonHEPEvtMessenger
                      (comptonHEPEvtInterface* iface)
: fHEPEvtInterface(iface)
{
  fDir = new G4UIdirectory("/generator/HEPEvt/");
  fDir->SetGuidance("Reading HEPEvt event from an Ascii file");

  fVerbose =
    new G4UIcmdWithAnInteger("/generator/HEPEvt/verbose", this);
  fVerbose->SetGuidance("Set verbose level");
  fVerbose->SetParameterName("verboseLevel", false, false);
  fVerbose->SetRange("verboseLevel>=0 && verboseLevel<=1");

  fOpen = new G4UIcmdWithAString("/generator/HEPEvt/open", this);
  fOpen->SetGuidance("(re)open data file (HEPEvt Ascii format)");
  fOpen->SetParameterName("input ascii file", true, true);
}

comptonHEPEvtMessenger::~comptonHEPEvtMessenger()
{
  delete fVerbose;
  delete fOpen;
  delete fDir;
}

void comptonHEPEvtMessenger::SetNewValue(G4UIcommand* command, G4String newValues)
{
  if (command == fVerbose) {
    int level = fVerbose->GetNewIntValue(newValues);
    fHEPEvtInterface->SetVerboseLevel(level);
  } else if (command == fOpen) {
    fHEPEvtInterface->SetFileName(newValues);
    G4cout << "HEPEvt Ascii inputfile: "
           << fHEPEvtInterface->GetFileName() << G4endl;
    fHEPEvtInterface->Initialize();
  }
}

G4String comptonHEPEvtMessenger::GetCurrentValue(G4UIcommand* command)
{
  G4String cv;
  if (command == fVerbose) {
    cv = fVerbose->ConvertToString(fHEPEvtInterface->GetVerboseLevel());
  } else if (command == fOpen) {
    cv = fHEPEvtInterface->GetFileName();
  }
  return cv;
}
