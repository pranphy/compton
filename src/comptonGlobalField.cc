#include "comptonGlobalField.hh"

#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4UImanager.hh"

#include "G4PropagatorInField.hh"

#include "G4Mag_UsualEqRhs.hh"
#include "G4EqMagElectricField.hh"
#include "G4Mag_SpinEqRhs.hh"
#include "G4EqEMFieldWithSpin.hh"

#include "G4ExplicitEuler.hh"
#include "G4ImplicitEuler.hh"
#include "G4SimpleRunge.hh"
#include "G4SimpleHeum.hh"
#include "G4ClassicalRK4.hh"
#include "G4CashKarpRKF45.hh"

#include "comptonMagneticField.hh"
#include "comptonSystemOfUnits.hh"

#include <comptontypes.hh>
#include <comptonRun.hh>
#include <comptonRunData.hh>

#include <TMD5.h>
#include <sys/stat.h>

#include <stdio.h>

#define __GLOBAL_NDIM 3

#include "G4Threading.hh"
#include "G4AutoLock.hh"
namespace { G4Mutex comptonGlobalFieldMutex = G4MUTEX_INITIALIZER; }

std::vector<comptonMagneticField*> comptonGlobalField::fFields;

comptonGlobalField::comptonGlobalField()
// NOTE: when changing defaults below, also change guidance in messenger commands
: fEquationType(0),fStepperType(4),
  fMinStep(0.01*mm),fDeltaChord(3.0*mm),
  fDeltaOneStep(0.01*mm),fDeltaIntersection(0.1*mm),
  fEpsMin(1.0e-5*mm),fEpsMax(1.0e-4*mm),
  fEquation(0),fEquationDoF(0),
  fFieldManager(0),fFieldPropagator(0),
  fStepper(0),fChordFinder(0),
  fVerboseLevel(0)
{
    // Get field propagator and managers
    G4TransportationManager* transportationmanager = G4TransportationManager::GetTransportationManager();
    fFieldPropagator = transportationmanager->GetPropagatorInField();
    fFieldManager = transportationmanager->GetFieldManager();

    // Create equation, stepper, and chordfinder
    SetEquation();
    SetStepper();
    SetChordFinder();
    SetAccuracyParameters();

    // Connect field manager to this global field
    fFieldManager->SetDetectorField(this);

    // Create generic messenger
    fMessenger.DeclareMethod("addfield",&comptonGlobalField::AddNewField,"Add magnetic field");

    // Create global field messenger
    fGlobalFieldMessenger.DeclareMethod("equationtype",&comptonGlobalField::SetEquationType,"Set equation type: \n 0: B-field, no spin (default); \n 2: B-field, with spin");
    fGlobalFieldMessenger.DeclareMethod("steppertype",&comptonGlobalField::SetStepperType,"Set stepper type: \n 0: ExplicitEuler; \n 1: ImplicitEuler; \n 2: SimpleRunge; \n 3: SimpleHeum; \n 4: ClassicalRK4 (default); \n 5: CashKarpRKF45");
    fGlobalFieldMessenger.DeclareMethod("print",&comptonGlobalField::PrintAccuracyParameters,"Print the accuracy parameters");
    fGlobalFieldMessenger.DeclareProperty("epsmin",fEpsMin,"Set the minimum epsilon of the field propagator");
    fGlobalFieldMessenger.DeclareProperty("epsmax",fEpsMax,"Set the maximum epsilon of the field propagator");
    fGlobalFieldMessenger.DeclareProperty("minstep",fMinStep,"Set the minimum step of the chord finder");
    fGlobalFieldMessenger.DeclareProperty("deltachord",fDeltaChord,"Set delta chord for the chord finder");
    fGlobalFieldMessenger.DeclareProperty("deltaonestep",fDeltaOneStep,"Set delta one step for the field manager");
    fGlobalFieldMessenger.DeclareProperty("deltaintersection",fMinStep,"Set delta intersection for the field manager");
    fGlobalFieldMessenger.DeclareMethod("interpolation",&comptonGlobalField::SetInterpolationType,"Set magnetic field interpolation type");
    //fGlobalFieldMessenger.DeclareMethod("zoffset",&comptonGlobalField::SetZOffset,"Set magnetic field z offset");
    fGlobalFieldMessenger.DeclareMethod("scale",&comptonGlobalField::SetFieldScale,"Scale magnetic field by factor");
    fGlobalFieldMessenger.DeclareMethod("current",&comptonGlobalField::SetMagnetCurrent,"Scale magnetic field by current");
    fGlobalFieldMessenger.DeclareMethod("value",&comptonGlobalField::PrintFieldValue,"Print the field value at a given point (in m)");
    fGlobalFieldMessenger.DeclareProperty("verbose",fVerboseLevel,"Set the verbose level");
}

comptonGlobalField::~comptonGlobalField()
{
         delete fEquation;
          delete fStepper;
      delete fChordFinder;
}

void comptonGlobalField::SetAccuracyParameters()
{
  // Set accuracy parameters
  fChordFinder->SetDeltaChord(fDeltaChord);

  fFieldManager->SetAccuraciesWithDeltaOneStep(fDeltaOneStep);
  fFieldManager->SetDeltaIntersection(fDeltaIntersection);

  fFieldPropagator->SetMinimumEpsilonStep(fEpsMin);
  fFieldPropagator->SetMaximumEpsilonStep(fEpsMax);
}

void comptonGlobalField::PrintAccuracyParameters()
{
  G4cout << "Accuracy Parameters:" <<
            " MinStep = " << fMinStep <<
            " DeltaChord = " << fDeltaChord <<
            " DeltaOneStep = " << fDeltaOneStep << G4endl;
  G4cout << "                    " <<
            " DeltaIntersection = " << fDeltaIntersection <<
            " EpsMin = " << fEpsMin <<
            " EpsMax = " << fEpsMax <<  G4endl;
}

void comptonGlobalField::SetEquation()
{
  delete fEquation;

  switch (fEquationType)
  {
    case 0:
      if (fVerboseLevel > 0) G4cout << "G4Mag_UsualEqRhs is called with 6 dof" << G4endl;
      fEquation = new G4Mag_UsualEqRhs(this);
      fEquationDoF = 6;
      break;
    case 2:
      if (fVerboseLevel > 0) G4cout << "G4Mag_SpinEqRhs is called with 12 dof" << G4endl;
      fEquation = new G4Mag_SpinEqRhs(this);
      fEquationDoF = 12;
      break;
    default: fEquation = 0;
  }

  SetStepper();
}

void comptonGlobalField::SetStepper()
{
  delete fStepper;

  switch (fStepperType)
  {
    case 0:
      fStepper = new G4ExplicitEuler(fEquation, fEquationDoF);
      if (fVerboseLevel > 0) G4cout << "G4ExplicitEuler is called" << G4endl;
      break;
    case 1:
      fStepper = new G4ImplicitEuler(fEquation, fEquationDoF);
      if (fVerboseLevel > 0) G4cout << "G4ImplicitEuler is called" << G4endl;
      break;
    case 2:
      fStepper = new G4SimpleRunge(fEquation, fEquationDoF);
      if (fVerboseLevel > 0) G4cout << "G4SimpleRunge is called" << G4endl;
      break;
    case 3:
      fStepper = new G4SimpleHeum(fEquation, fEquationDoF);
      if (fVerboseLevel > 0) G4cout << "G4SimpleHeum is called" << G4endl;
      break;
    case 4:
      fStepper = new G4ClassicalRK4(fEquation, fEquationDoF);
      if (fVerboseLevel > 0) G4cout << "G4ClassicalRK4 (default) is called" << G4endl;
      break;
    case 5:
      fStepper = new G4CashKarpRKF45(fEquation, fEquationDoF);
      if (fVerboseLevel > 0) G4cout << "G4CashKarpRKF45 is called" << G4endl;
      break;
    default: fStepper = 0;
  }

  SetChordFinder();
}

void comptonGlobalField::SetChordFinder()
{
  delete fChordFinder;

  fChordFinder = new G4ChordFinder(this,fMinStep,fStepper);
  fChordFinder->GetIntegrationDriver()->SetVerboseLevel(0);
  fFieldManager->SetChordFinder(fChordFinder);
}

void comptonGlobalField::AddNewField(G4String& name)
{
  // Lock mutex to ensure only 1 thread is loading a field
  G4AutoLock lock(&comptonGlobalFieldMutex);

  // If this field has already been loaded
  if (GetFieldByName(name) != 0) return;

  // Load new field
  comptonMagneticField *thisfield = new comptonMagneticField(name);
  fFields.push_back(thisfield);

  if (fVerboseLevel > 0)
    G4cout << __FUNCTION__ << ": field " << name << " was added." << G4endl;

  // Add file data to output data stream
  comptonRunData *rd = comptonRun::GetRunData();

  // FIXME disabled TMD5 functionality as long as CentOS 7.2 is common
  // due to kernel bug when running singularity containers

  //TMD5 *md5 = TMD5::FileChecksum(name.data());

  filedata_t fdata;

  strcpy(fdata.filename, name.data());
  strcpy(fdata.hashsum, "no hash" ); // md5->AsString() );

  //G4cout << "MD5 checksum " << md5->AsString() << G4endl;

  //delete md5;

  struct stat fs;
  stat(name.data(), &fs);
  fdata.timestamp = TTimeStamp( fs.st_mtime );

  if (fVerboseLevel > 0)
    G4cout << __FUNCTION__ << ": field timestamp = " << fdata.timestamp << G4endl;

  rd->AddMagData(fdata);
}

comptonMagneticField* comptonGlobalField::GetFieldByName(const G4String& name) const
{
    for (auto it = fFields.begin(); it != fFields.end(); it++)
        if ((*it)->GetName() == name)
          return (*it);

    //for(auto field: fFields) if (field->GetName() == name) return field;
    return 0;
}

void comptonGlobalField::PrintFieldValue(const G4ThreeVector& r)
{
    G4double B[__GLOBAL_NDIM];
    G4double p[] = {r.x()*m, r.y()*m, r.z()*m, 0.0};
    GetFieldValue(p, B);
    G4cout << "At r" << r << " [m]: B = ";
    for (int i = 0; i < __GLOBAL_NDIM; i++) {
        G4cout << B[i]/tesla << " ";
    }
    G4cout << "T" << G4endl;
}

void comptonGlobalField::GetFieldValue(const G4double p[], G4double *field) const
{
    // Field is not initialized to zero by geant4
    field[0] = 0.0;
    field[1] = 0.0;
    field[2] = 0.0;
    G4double k[3] = {0,0,0};
    //for (auto it = fFields.begin(); it != fFields.end(); it++){
    for(auto mfield: fFields){
        mfield->GetFieldValue(p,k);
        //if( k[0] != 0 ){
            //std::cout<<"For the "<<mfield->GetName();
            //std::cout<<"FFF, "<<p[0]<<","<<p[1]<<", "<<p[2]<<",   "<<k[0]<<","<<k[1]<<", "<<k[2]<<std::endl;
            //std::cout<<"For the "<<mfield->GetName()<<" got "<<k[0]<<","<<k[1]<<", "<<k[2]<<std::endl;
        //}
        mfield->AddFieldValue(p, field);
    }
}

/*
void comptonGlobalField::SetZOffset(const G4String& name, G4double offset)
{
  comptonMagneticField *field = GetFieldByName(name);
  if (field != nullptr) {
    G4AutoLock lock(&comptonGlobalFieldMutex);
    field->SetZoffset(offset);
  } else {
    G4cerr << "WARNING " << __FILE__ << " line " << __LINE__
           << ": field " << name << " offset failed" << G4endl;
  }
}
*/

void comptonGlobalField::SetInterpolationType(const G4String& name, const G4String& type)
{
  comptonMagneticField *field = GetFieldByName(name);
  if (field != nullptr) {
    G4AutoLock lock(&comptonGlobalFieldMutex);
    field->SetInterpolationType(type);
  } else {
    G4cerr << "WARNING " << __FILE__ << " line " << __LINE__
           << ": field " << name << " interpolation type failed" << G4endl;
  }
}

void comptonGlobalField::SetFieldScale(const G4String& name, G4double scale)
{
  comptonMagneticField *field = GetFieldByName(name);
  if (field != nullptr) {
    G4AutoLock lock(&comptonGlobalFieldMutex);
    field->SetFieldScale(scale);
  } else {
    G4cerr << "WARNING " << __FILE__ << " line " << __LINE__
           << ": field " << name << " scaling failed" << G4endl;
  }
}

void comptonGlobalField::SetMagnetCurrent(const G4String& name, G4double current)
{
  comptonMagneticField *field = GetFieldByName(name);
  if (field != nullptr) {
    G4AutoLock lock(&comptonGlobalFieldMutex);
    field->SetRefCurrent(current);
  } else {
    G4cerr << "WARNING " << __FILE__ << " line " << __LINE__
           << ": field " << name << " scaling failed" << G4endl;
  }
}
