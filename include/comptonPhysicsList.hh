#ifndef comptonPhysicsList_h
#define comptonPhysicsList_h 1

#include "G4VModularPhysicsList.hh"
#include "G4GenericMessenger.hh"

class G4VPhysicsConstructor;

class comptonPhysicsList: public G4VModularPhysicsList
{
  public:
    comptonPhysicsList();
    virtual ~comptonPhysicsList();

  public:
    // Set verbose level
    void SetVerboseLevel(G4int level);

    // Set Parallel physics
    void SetParallelPhysics(G4bool flag);
    // Enable Parallel physics
    void EnableParallelPhysics();
    // Disable Parallel physics
    void DisableParallelPhysics();

    // Set Synchrotron physics
    void SetSynchrotronPhysics(G4bool flag);
    // Enable Synchrotron physics
    void EnableSynchrotronPhysics();
    // Disable Synchrotron physics
    void DisableSynchrotronPhysics();

    // Set optical physics
    void SetOpticalPhysics(G4bool flag);
    // Enable optical physics
    void EnableOpticalPhysics();
    // Disable optical physics
    void DisableOpticalPhysics();

    // Set step limiter physics
    void SetStepLimiterPhysics(G4bool flag);
    // Enable step limiter physics
    void EnableStepLimiterPhysics();
    // Disable step limiter physics
    void DisableStepLimiterPhysics();

    // Handle reference physics lists in messenger
    void ListReferencePhysLists();
    void RemoveReferencePhysList();
    void RegisterReferencePhysList(G4String name);

  private:
    G4String fReferencePhysListName;
    G4VModularPhysicsList* fReferencePhysList;
    G4VPhysicsConstructor* fParallelPhysics;
    G4VPhysicsConstructor* fOpticalPhysics;
    G4VPhysicsConstructor* fStepLimiterPhysics;
    G4VPhysicsConstructor* fSynchrotronPhysics;

    // Deleting an unused physics list also deletes particles, causing
    // all kinds of issues with new reference physics lists
    std::vector<G4VModularPhysicsList*> fReferencePhysicsListToDelete;

  protected:
    // Generic messenger as protected to be used in derived classes
    G4GenericMessenger fPhysListMessenger{
      this,
      "/compton/physlist/",
      "Compton physics list properties"};
    G4GenericMessenger fOpticalMessenger{
      this,
      "/compton/physlist/optical/",
      "Compton optical physics properties"};
    G4GenericMessenger fParallelMessenger{
      this,
      "/compton/physlist/parallel/",
      "Compton parallel physics properties"};
    G4GenericMessenger fStepLimiterMessenger{
      this,
      "/compton/physlist/steplimiter/",
      "Compton step limiter properties"};

    G4GenericMessenger fSynchrotronMessenger{
      this,
      "/compton/physlist/synchrotron/",
      "Compton step limiter properties"};
};

#endif
