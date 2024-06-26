#include "comptonPhysicsList.hh"

#include "G4PhysListFactory.hh"
#include "G4ParallelWorldPhysics.hh"
#include "G4OpticalPhysics.hh"
#include "G4RunManager.hh"
#include "G4NuclearLevelData.hh"
#include "G4HadronicProcessStore.hh"
#include "G4ParticleHPManager.hh"

#include "G4Version.hh"
#if G4VERSION_NUMBER < 1000
#include "G4StepLimiterBuilder.hh"
#else
#include "G4StepLimiterPhysics.hh"
#endif

comptonPhysicsList::comptonPhysicsList()
: G4VModularPhysicsList(),
  fReferencePhysList(0),
  fParallelPhysics(0),
  fOpticalPhysics(0),
  fStepLimiterPhysics(0)
{
  // Let users know to ignore the warning by Particle HP package
  G4cout << "compton: Since the high precision neutron simulation in the some physics lists  " << G4endl;
  G4cout << "compton: generates a lot of warnings that cannot be avoided, we are setting the " << G4endl;
  G4cout << "compton: physics list verbose level to zero. Use /compton/physlist/verbose to set" << G4endl;
  G4cout << "compton: the verbose level to a non-zero value." << G4endl << G4endl;
  //
  SetVerboseLevel(0);

  // Set and print default reference physics list
  //RegisterReferencePhysList("QGSP_BERT");
  RegisterReferencePhysList("FTFP_BERT_EMZ");
  G4cout << "compton: loaded reference physics list " << fReferencePhysListName << G4endl;

  // Set and print default status of other physics
  EnableStepLimiterPhysics();
  EnableParallelPhysics();
  DisableOpticalPhysics();
  G4cout << "compton: step limiter physics is " << (fStepLimiterPhysics != nullptr? "enabled":"disabled") << G4endl;
  G4cout << "compton: parallel physics is "     << (fParallelPhysics != nullptr?    "enabled":"disabled") << G4endl;
  G4cout << "compton: optical physics is "      << (fOpticalPhysics != nullptr?     "enabled":"disabled") << G4endl;

  // Create commands
  fPhysListMessenger.DeclareMethod(
      "verbose",
      &comptonPhysicsList::SetVerboseLevel,
      "Set physics list verbose level")
              .SetStates(G4State_PreInit);
  fPhysListMessenger.DeclareMethod(
      "register",
      &comptonPhysicsList::RegisterReferencePhysList,
      "Register reference physics list")
              .SetStates(G4State_PreInit);
  fPhysListMessenger.DeclareMethod(
      "list",
      &comptonPhysicsList::ListReferencePhysLists,
      "List reference physics lists");

  fParallelMessenger.DeclareMethod(
      "enable",
      &comptonPhysicsList::EnableParallelPhysics,
      "Enable parallel physics")
              .SetStates(G4State_PreInit);
  fParallelMessenger.DeclareMethod(
      "disable",
      &comptonPhysicsList::DisableParallelPhysics,
      "Disable parallel physics")
              .SetStates(G4State_PreInit);

  fOpticalMessenger.DeclareMethod(
      "enable",
      &comptonPhysicsList::EnableOpticalPhysics,
      "Enable optical physics")
              .SetStates(G4State_PreInit);
  fOpticalMessenger.DeclareMethod(
      "disable",
      &comptonPhysicsList::DisableOpticalPhysics,
      "Disable optical physics")
              .SetStates(G4State_PreInit);

  fStepLimiterMessenger.DeclareMethod(
      "enable",
      &comptonPhysicsList::EnableStepLimiterPhysics,
      "Enable step limiter");
  fStepLimiterMessenger.DeclareMethod(
      "disable",
      &comptonPhysicsList::DisableStepLimiterPhysics,
      "Disable step limiter");
}

comptonPhysicsList::~comptonPhysicsList()
{
  // TODO deleting old reference physics lists still fails when the same one
  // was loaded again, e.g. when loading default physics list explicitly
  //for (size_t i = 0; i < fReferencePhysicsListToDelete.size(); i++) {
  //  delete fReferencePhysicsListToDelete.at(i);
  //}
  //fReferencePhysicsListToDelete.clear();
}

void comptonPhysicsList::SetVerboseLevel(G4int level)
{
  // Let upstream handle this first
  G4VModularPhysicsList::SetVerboseLevel(level);

  // Set verbose level of precompound deexcitation
  #if G4VERSION_NUMBER >= 1060
  if (auto nuclearleveldata = G4NuclearLevelData::GetInstance())
    if (auto param = nuclearleveldata->GetParameters())
      param->SetVerbose(level);
  #endif

  // Set verbose level of HadronicProcessStore
  G4HadronicProcessStore::Instance()->SetVerbose(level);
  G4ParticleHPManager::GetInstance()->SetVerboseLevel(level);
  G4cout << G4endl; // empty line after G4ParticleHPManager complaint
}

void comptonPhysicsList::SetParallelPhysics(G4bool flag)
{
  if (flag) EnableParallelPhysics();
  else     DisableParallelPhysics();
}

void comptonPhysicsList::EnableParallelPhysics()
{
  if (fParallelPhysics != nullptr) {
    G4cout << "Parallel physics already active" << G4endl;
    return;
  }

  // Print output
  if (GetVerboseLevel() > 0)
    G4cout << "Registering parallel physics" << G4endl;

  // Create Parallel physics
  fParallelPhysics = new G4ParallelWorldPhysics("parallel");
  // Note: name must correspond with name of comptonParallelConstruction

  // Register existing physics
  RegisterPhysics(fParallelPhysics);
}

void comptonPhysicsList::DisableParallelPhysics()
{
  if (fParallelPhysics == nullptr) {
    G4cout << "Parallel physics not active" << G4endl;
    return;
  }

  // Print output
  if (GetVerboseLevel() > 0)
    G4cout << "Removing parallel physics" << G4endl;

  // Remove Parallel physics
  RemovePhysics(fParallelPhysics);

  // Delete Parallel physics
  delete fParallelPhysics;
  fParallelPhysics = 0;
}

void comptonPhysicsList::SetOpticalPhysics(G4bool flag)
{
  if (flag) EnableOpticalPhysics();
  else     DisableOpticalPhysics();
}

void comptonPhysicsList::EnableOpticalPhysics()
{
  if (fOpticalPhysics != nullptr) {
    G4cout << "Optical physics already active" << G4endl;
    return;
  }

  // Print output
  if (GetVerboseLevel() > 0)
    G4cout << "Registering optical physics" << G4endl;

  // Create optical physics
  fOpticalPhysics = new G4OpticalPhysics(GetVerboseLevel());

  // Register existing physics
  RegisterPhysics(fOpticalPhysics);
}

void comptonPhysicsList::DisableOpticalPhysics()
{
  if (fOpticalPhysics == nullptr) {
    G4cout << "Optical physics not active" << G4endl;
    return;
  }

  // Print output
  if (GetVerboseLevel() > 0)
    G4cout << "Removing optical physics" << G4endl;

  // Remove optical physics
  RemovePhysics(fOpticalPhysics);

  // Delete optical physics
  delete fOpticalPhysics;
  fOpticalPhysics = 0;
}

void comptonPhysicsList::SetStepLimiterPhysics(G4bool flag)
{
  if (flag) EnableStepLimiterPhysics();
  else     DisableStepLimiterPhysics();
}

void comptonPhysicsList::EnableStepLimiterPhysics()
{
  if (fStepLimiterPhysics != nullptr) {
    G4cout << "Step limiter already active" << G4endl;
    return;
  }

  // Print output
  if (GetVerboseLevel() > 0)
    G4cout << "Registering step limiter physics" << G4endl;

  // Create step limiter physics
  #if G4VERSION_NUMBER < 1000
  fStepLimiterPhysics = new G4StepLimiterBuilder(GetVerboseLevel());
  #elif G4VERSION_NUMBER < 1100
  fStepLimiterPhysics = new G4StepLimiterPhysics(GetVerboseLevel());
  #else
  fStepLimiterPhysics = new G4StepLimiterPhysics();
  fStepLimiterPhysics->SetVerboseLevel(GetVerboseLevel());
  #endif

  // Register existing physics
  RegisterPhysics(fStepLimiterPhysics);
}

void comptonPhysicsList::DisableStepLimiterPhysics()
{
  if (fStepLimiterPhysics == nullptr) {
    G4cout << "Step limiter physics not active" << G4endl;
    return;
  }

  // Print output
  if (GetVerboseLevel() > 0)
    G4cout << "Removing step limiter physics" << G4endl;

  // Remove step limiter physics
  RemovePhysics(fStepLimiterPhysics);

  // Delete step limiter physics
  delete fStepLimiterPhysics;
  fStepLimiterPhysics = 0;
}

void comptonPhysicsList::ListReferencePhysLists()
{
  G4PhysListFactory factory;
  std::vector<G4String> list;

  G4cout << "Available reference physics lists:" << G4endl;
  list = factory.AvailablePhysLists();
  for (std::vector<G4String>::const_iterator
      item  = list.begin();
      item != list.end();
      item++)
    G4cout << " " << *item << G4endl;
  G4cout << G4endl;

  G4cout << "Available physics list EM options:" << G4endl;
  list = factory.AvailablePhysListsEM();
  for (std::vector<G4String>::const_iterator
      item  = list.begin();
      item != list.end();
      item++)
    G4cout << " " << *item << G4endl;
  G4cout << G4endl;
}

void comptonPhysicsList::RemoveReferencePhysList()
{
  // Print output
  if (GetVerboseLevel() > 0)
    G4cout << "Removing existing reference physics list" << G4endl;

  // Remove physics in previous reference list
  G4int i = 0;
  G4VPhysicsConstructor* elem = 0;
  while ((elem = const_cast<G4VPhysicsConstructor*>(fReferencePhysList->GetPhysics(i++))) != 0) {
    // Print output
    if (GetVerboseLevel() > 0)
      G4cout << "Removing " << elem->GetPhysicsName() << G4endl;

    // Remove physics
    RemovePhysics(elem);
  }

  // Delete reference physics list
  fReferencePhysicsListToDelete.push_back(fReferencePhysList);
  fReferencePhysList = 0;
}

void comptonPhysicsList::RegisterReferencePhysList(G4String name)
{
  // Already loaded
  if (name == fReferencePhysListName) {
    G4cout << "Reference physics list " << name << " already loaded" << G4endl;
    return;
  }

  // Load the factory
  G4PhysListFactory factory;
  factory.SetVerbose(GetVerboseLevel());

  // Check whether this reference physics list exists
  if (! factory.IsReferencePhysList(name)) {
    G4cerr << "Physics list " << name
           << " is not a valid reference physics list" << G4endl;
    ListReferencePhysLists();
    return;
  }

  // Remove previous reference physics list
  if (fReferencePhysList != nullptr) RemoveReferencePhysList();

  // Get reference physics list
  fReferencePhysList = factory.GetReferencePhysList(name);
  fReferencePhysList->SetVerboseLevel(GetVerboseLevel());
  fReferencePhysListName = name;

  // Register physics from this list
  G4int i = 0;
  G4VPhysicsConstructor* elem = 0;
  while ((elem = const_cast<G4VPhysicsConstructor*>(fReferencePhysList->GetPhysics(i++))) != 0) {
    // Change verbose level
    elem->SetVerboseLevel(GetVerboseLevel());

    // Print output
    if (GetVerboseLevel() > 0)
      G4cout << "Registering " << elem->GetPhysicsName() << G4endl;

    // Register existing physics
    RegisterPhysics(elem);
  }

  // Blank space
  if (GetVerboseLevel() > 0)
    G4cout << G4endl;
}
