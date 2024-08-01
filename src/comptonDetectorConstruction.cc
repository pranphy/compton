#include "comptonDetectorConstruction.hh"

#include "comptonGenericDetector.hh"
#include "comptonBeamTarget.hh"
#include "comptonGlobalField.hh"
#include "comptonIO.hh"

#include "G4GeometryManager.hh"
#include "G4GeometryTolerance.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UserLimits.hh"

#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "globals.hh"

#include "G4RunManager.hh"

#include "G4SDManager.hh"
#include "G4VSensitiveDetector.hh"

#include "G4UnitsTable.hh"
#include "G4NistManager.hh"

// GDML export
#include "G4GDMLParser.hh"

//CADMesh
#ifdef __USE_CADMESH
#include <CADMesh.hh>
#endif

// visual
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include <algorithm>
#include <sys/param.h>

#ifdef __APPLE__
#include <unistd.h>
#endif

#include "G4Threading.hh"
#include "G4AutoLock.hh"
namespace { G4Mutex comptonDetectorConstructionMutex = G4MUTEX_INITIALIZER; }

G4ThreadLocal comptonGlobalField* comptonDetectorConstruction::fGlobalField = 0;

G4UserLimits* comptonDetectorConstruction::fKryptoniteUserLimits = new G4UserLimits(0,0,0,DBL_MAX,DBL_MAX);

comptonDetectorConstruction::comptonDetectorConstruction(const G4String& name, const G4String& gdmlfile)
: fVerboseLevel(0),
  fGDMLValidate(false),
  fGDMLOverlapCheck(true),
  fGDMLPath(""),
  fGDMLFile(""),
  fKryptoniteEnable(true),
  fKryptoniteVerbose(0),
  fWorldVolume(0),
  fWorldName(name)
{
    std::cout<<"Also now calling this"<<name<<" and "<<gdmlfile<<std::endl;
  // Define some engineering units
  new G4UnitDefinition("inch","in","Length",25.4*CLHEP::millimeter);

  SetGDMLFile("geometries/ComptonGeometry.gdml");
  // If gdmlfile is non-empty
  if (gdmlfile.length() > 0) {
      SetGDMLFile(gdmlfile);
  }

  // New units
  new G4UnitDefinition("inch","in","Length",25.4*CLHEP::millimeter);

  // Create generic messenger
  fMessenger.DeclareMethod(
      "setgeofile",
      &comptonDetectorConstruction::SetGDMLFile,
      "Set geometry GDML file")
      .SetStates(G4State_PreInit);
  fMessenger.DeclareMethod(
      "printgeometry",
      &comptonDetectorConstruction::PrintGeometry,
      "Print the geometry tree")
      .SetStates(G4State_Idle)
      .SetDefaultValue("false");
  fMessenger.DeclareMethod(
      "printelements",
      &comptonDetectorConstruction::PrintElements,
      "Print the elements")
      .SetStates(G4State_Idle);
  fMessenger.DeclareMethod(
      "printmaterials",
      &comptonDetectorConstruction::PrintMaterials,
      "Print the materials")
      .SetStates(G4State_Idle);

  // Create geometry messenger
  fGeometryMessenger.DeclareMethod(
      "setfile",
      &comptonDetectorConstruction::SetGDMLFile,
      "Set geometry GDML file")
      .SetStates(G4State_PreInit);
  fGeometryMessenger.DeclareProperty(
      "verbose",
      fVerboseLevel,
      "Set geometry verbose level")
          .SetStates(G4State_PreInit,G4State_Idle);
  fGeometryMessenger.DeclareProperty(
      "validate",
      fGDMLValidate,
      "Set GMDL validate flag")
          .SetStates(G4State_PreInit)
          .SetDefaultValue("true");
  fGeometryMessenger.DeclareProperty(
      "overlapcheck",
      fGDMLOverlapCheck,
      "Set GMDL overlap check flag")
          .SetStates(G4State_PreInit)
          .SetDefaultValue("true");
  fGeometryMessenger.DeclareMethod(
      "load",
      &comptonDetectorConstruction::ReloadGeometry,
      "Reload the geometry")
      .SetStates(G4State_PreInit,G4State_Idle);
  fGeometryMessenger.DeclareMethod(
      "printelements",
      &comptonDetectorConstruction::PrintElements,
      "Print the elements")
      .SetStates(G4State_Idle);
  fGeometryMessenger.DeclareMethod(
      "printmaterials",
      &comptonDetectorConstruction::PrintMaterials,
      "Print the materials")
      .SetStates(G4State_Idle);
  fGeometryMessenger.DeclareMethod(
      "printgeometry",
      &comptonDetectorConstruction::PrintGeometry,
      "Print the geometry tree")
      .SetStates(G4State_Idle)
      .SetDefaultValue("false");
  fGeometryMessenger.DeclareMethod(
      "printoverlaps",
      &comptonDetectorConstruction::PrintOverlaps,
      "Print the geometry overlap")
      .SetStates(G4State_Idle);
  fGeometryMessenger.DeclareMethod(
      "absolute_position",
      &comptonDetectorConstruction::AbsolutePosition,
      "Set the position of volume in parent frame [mm]")
      .SetStates(G4State_PreInit,G4State_Idle);
  fGeometryMessenger.DeclareMethod(
      "relative_position",
      &comptonDetectorConstruction::RelativePosition,
      "Position a volume relative to current position [mm]")
      .SetStates(G4State_PreInit,G4State_Idle);
  fGeometryMessenger.DeclareMethod(
      "absolute_rotation",
      &comptonDetectorConstruction::AbsoluteRotation,
      "Set the rotation of volume in parent frame [deg]")
      .SetStates(G4State_PreInit,G4State_Idle);
  fGeometryMessenger.DeclareMethod(
      "relative_rotation",
      &comptonDetectorConstruction::RelativeRotation,
      "Rotate a volume relative to current orientation [deg]")
      .SetStates(G4State_PreInit,G4State_Idle);
  fGeometryMessenger.DeclareMethod(
      "addmesh",
      &comptonDetectorConstruction::AddMesh,
      "Add mesh file (ascii stl, ascii ply, ascii obj)")
      .SetStates(G4State_Idle);

  // Create user limits messenger
  fUserLimitsMessenger.DeclareMethod(
      "usermaxallowedstep",
      &comptonDetectorConstruction::SetUserMaxAllowedStep,
      "Set user limit MaxAllowedStep for logical volume")
      .SetStates(G4State_Idle);
  fUserLimitsMessenger.DeclareMethod(
      "usermaxtracklength",
      &comptonDetectorConstruction::SetUserMaxTrackLength,
      "Set user limit MaxTrackLength for logical volume")
      .SetStates(G4State_Idle);
  fUserLimitsMessenger.DeclareMethod(
      "usermaxtime",
      &comptonDetectorConstruction::SetUserMaxTime,
      "Set user limit MaxTime for logical volume")
      .SetStates(G4State_Idle);
  fUserLimitsMessenger.DeclareMethod(
      "userminekine",
      &comptonDetectorConstruction::SetUserMinEkine,
      "Set user limit MinEkine for logical volume")
      .SetStates(G4State_Idle);
  fUserLimitsMessenger.DeclareMethod(
      "userminrange",
      &comptonDetectorConstruction::SetUserMinRange,
      "Set user limit MinRange for logical volume")
      .SetStates(G4State_Idle);

  // Create kryptonite messenger
  fKryptoniteMessenger.DeclareMethod(
      "verbose",
      &comptonDetectorConstruction::SetKryptoniteVerbose,
      "Set verbose level");
  fKryptoniteMessenger.DeclareMethod(
      "enable",
      &comptonDetectorConstruction::EnableKryptonite,
      "Treat materials as kryptonite");
  fKryptoniteMessenger.DeclareMethod(
      "disable",
      &comptonDetectorConstruction::DisableKryptonite,
      "Treat materials as regular");
  fKryptoniteMessenger.DeclareMethod(
      "add",
      &comptonDetectorConstruction::AddKryptoniteCandidate,
      "Add specified material to list of kryptonite candidates");
  fKryptoniteMessenger.DeclareMethod(
      "list",
      &comptonDetectorConstruction::ListKryptoniteCandidates,
      "List kryptonite candidate materials");
  fKryptoniteMessenger.DeclareMethod(
      "volume",
      &comptonDetectorConstruction::EnableKryptoniteVolume,
      "Treat volume as kryptonite");
}



void comptonDetectorConstruction::EnableKryptonite()
{
  if (fKryptoniteVerbose > 0)
    G4cout << "Enabling kryptonite." << G4endl;

  fKryptoniteEnable = true;

  SetKryptoniteUserLimits(fWorldVolume);
}

void comptonDetectorConstruction::DisableKryptonite()
{
  if (fKryptoniteVerbose > 0)
    G4cout << "Disabling kryptonite." << G4endl;

  fKryptoniteEnable = false;

  SetKryptoniteUserLimits(fWorldVolume);
}

void comptonDetectorConstruction::EnableKryptoniteVolume(G4String name)
{
  if (fKryptoniteVerbose > 0)
    G4cout << "Enabling kryptonite on volume" << name << "." << G4endl;

  // Find volume
  G4LogicalVolume* logical_volume = G4LogicalVolumeStore::GetInstance()->GetVolume(name);
  if (logical_volume == nullptr) {
    G4cerr << __FILE__ << " line " << __LINE__ << ": Warning volume " << name << " unknown" << G4endl;
    return;
  }

  fKryptoniteEnable = true;

  logical_volume->SetUserLimits(fKryptoniteUserLimits);
}

void comptonDetectorConstruction::AddKryptoniteCandidate(G4String name)
{
  if (fKryptoniteVerbose > 0)
    G4cout << "Adding " << name << " to list of kryptonite candidates." << G4endl;

  fKryptoniteCandidates.insert(name);
  InitKryptoniteMaterials();

  SetKryptoniteUserLimits(fWorldVolume);
}

void comptonDetectorConstruction::ListKryptoniteCandidates()
{
  G4cout << "List of kryptonite candidate materials:" << G4endl;
  for (std::set<G4String>::const_iterator
      it  = fKryptoniteCandidates.begin();
      it != fKryptoniteCandidates.end();
      it++)
    G4cout << *it << G4endl;
}

void comptonDetectorConstruction::InitKryptoniteMaterials()
{
  if (fKryptoniteVerbose > 0)
    G4cout << "Regenerating table of kryptonite material candidate pointers..." << G4endl;

  // Find kryptonite materials in material tables
  G4MaterialTable* table = G4Material::GetMaterialTable();
  fKryptoniteMaterials.clear();
  for (G4MaterialTable::const_iterator
      it  = table->begin();
      it != table->end(); it++) {
    if (fKryptoniteCandidates.find((*it)->GetName()) != fKryptoniteCandidates.end()) {
      fKryptoniteMaterials.insert(*it);
    }
  }
}

void comptonDetectorConstruction::SetKryptoniteUserLimits(G4VPhysicalVolume* volume)
{
  // If null volume, pick entire world
  if (volume == 0) volume = fWorldVolume;
  // If still null, give up
  if (volume == 0) return;

  // Get logical volume
  G4LogicalVolume* logical_volume = volume->GetLogicalVolume();
  G4Material* material = logical_volume->GetMaterial();

  // Set user limits for all materials in kryptonite materials list
  if (fKryptoniteMaterials.count(material) > 0) {
    if (fKryptoniteVerbose > 0)
      G4cout << "Setting kryptonite for " << logical_volume->GetName() << " to " <<
        (fKryptoniteEnable?"on":"off") << G4endl;

    if (fKryptoniteEnable)
      logical_volume->SetUserLimits(fKryptoniteUserLimits);
    else
      logical_volume->SetUserLimits(0);
  }

  // Descend down the tree
  auto n = logical_volume->GetNoDaughters();
  for (decltype(n) i = 0; i < n; i++) {
    G4VPhysicalVolume* daughter = logical_volume->GetDaughter(i);
    SetKryptoniteUserLimits(daughter);
  }
}


// Set of functions that passes function name as string for further processing
void comptonDetectorConstruction::SetUserMaxAllowedStep(G4String name, G4String value_units)
{
  SetUserLimits(__FUNCTION__,name,value_units);
}
void comptonDetectorConstruction::SetUserMaxTrackLength(G4String name, G4String value_units)
{
  SetUserLimits(__FUNCTION__,name,value_units);
}
void comptonDetectorConstruction::SetUserMaxTime(G4String name, G4String value_units)
{
  SetUserLimits(__FUNCTION__,name,value_units);
}
void comptonDetectorConstruction::SetUserMinEkine(G4String name, G4String value_units)
{
  SetUserLimits(__FUNCTION__,name,value_units);
}
void comptonDetectorConstruction::SetUserMinRange(G4String name, G4String value_units)
{
  SetUserLimits(__FUNCTION__,name,value_units);
}

comptonDetectorConstruction::~comptonDetectorConstruction()
{
    for (auto pv: fMeshPVs) {
      auto lv = pv->GetLogicalVolume();
      auto solid = lv->GetSolid();
      delete solid;
      delete lv;
      delete pv;
    }
}

void comptonDetectorConstruction::AddMesh(const G4String& filename)
{
  #ifdef __USE_CADMESH
    // Read mesh
    auto mesh = CADMesh::TessellatedMesh::FromSTL(filename);

    // Extract solids
    G4Material* material = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
    for (auto solid: mesh->GetSolids()) {
      auto lv = new G4LogicalVolume(solid, material, filename);
      lv->SetVisAttributes(G4Colour(0.0,1.0,0.0,1.0));
      auto pv = new G4PVPlacement(G4Transform3D(), filename, lv, fWorldVolume, false, 0, false);
      fMeshPVs.push_back(pv);
    }

    // Reoptimize geometry
    G4RunManager* run_manager = G4RunManager::GetRunManager();
    run_manager->GeometryHasBeenModified();
  #else
    G4cerr << __FILE__ << " line " << __LINE__ << ": Warning - meshes not supported." << G4endl;
  #endif
}

void comptonDetectorConstruction::AbsolutePosition(G4String name, G4ThreeVector position)
{
  // Units
  position *= CLHEP::mm;

  // Find volume
  G4VPhysicalVolume* physical_volume = G4PhysicalVolumeStore::GetInstance()->GetVolume(name);
  if (physical_volume == nullptr) {
    G4cerr << __FILE__ << " line " << __LINE__ << ": Warning volume " << name << " unknown" << G4endl;
    return;
  }

  // Print verbose
  if (fVerboseLevel > 0)
    G4cout << "Setting position in mother volume "
           << "from " << physical_volume->GetTranslation() << " "
           << "to " << position << " for " << name << G4endl;

  // Set position for volume
  physical_volume->SetTranslation(position);

  // Reoptimize geometry
  G4RunManager* run_manager = G4RunManager::GetRunManager();
  run_manager->GeometryHasBeenModified();
}

void comptonDetectorConstruction::RelativePosition(G4String name, G4ThreeVector position)
{
  // Units
  position *= CLHEP::mm;

  // Find volume
  G4VPhysicalVolume* physical_volume = G4PhysicalVolumeStore::GetInstance()->GetVolume(name);
  if (physical_volume == nullptr) {
    G4cerr << __FILE__ << " line " << __LINE__ << ": Warning volume " << name << " unknown" << G4endl;
    return;
  }

  // Print verbose
  if (fVerboseLevel > 0)
    G4cout << "Changing position in mother volume "
           << "from " << physical_volume->GetTranslation() << " "
           << "by " << position << " for " << name << G4endl;

  // Set position for volume
  physical_volume->SetTranslation(physical_volume->GetTranslation() + position);

  // Reoptimize geometry
  G4RunManager* run_manager = G4RunManager::GetRunManager();
  run_manager->GeometryHasBeenModified();
}

void comptonDetectorConstruction::AbsoluteRotation(G4String name, G4ThreeVector rotation_xyz)
{
  // Units
  rotation_xyz *= CLHEP::deg;

  // Find volume
  G4VPhysicalVolume* physical_volume = G4PhysicalVolumeStore::GetInstance()->GetVolume(name);
  if (physical_volume == nullptr) {
    G4cerr << __FILE__ << " line " << __LINE__ << ": Warning volume " << name << " unknown" << G4endl;
    return;
  }

  // Construct rotation matrix
  G4RotationMatrix* rotation = new G4RotationMatrix();
  rotation->rotateX(rotation_xyz.x());
  rotation->rotateY(rotation_xyz.y());
  rotation->rotateZ(rotation_xyz.z());

  // Get previous rotation matrix
  G4RotationMatrix* old_rotation = physical_volume->GetRotation();
  if (old_rotation == 0) old_rotation = new G4RotationMatrix();

  // Print verbose
  if (fVerboseLevel > 0)
    G4cout << "Setting rotation in mother volume "
           << "from " << *old_rotation << " "
           << "to " << *rotation << " for " << name << G4endl;

  // Set position for volume
  physical_volume->SetRotation(rotation);

  // Delete old rotation matrix
  delete old_rotation;

  // Reoptimize geometry
  G4RunManager* run_manager = G4RunManager::GetRunManager();
  run_manager->GeometryHasBeenModified();
}

void comptonDetectorConstruction::RelativeRotation(G4String name, G4ThreeVector rotation_xyz)
{
  // Units
  rotation_xyz *= CLHEP::deg;

  // Find volume
  G4VPhysicalVolume* physical_volume = G4PhysicalVolumeStore::GetInstance()->GetVolume(name);
  if (physical_volume == nullptr) {
    G4cerr << __FILE__ << " line " << __LINE__ << ": Warning volume " << name << " unknown" << G4endl;
    return;
  }

  // Get previous rotation matrix
  G4RotationMatrix* old_rotation = physical_volume->GetRotation();
  if (old_rotation == 0) old_rotation = new G4RotationMatrix();

  // Apply relative rotation
  G4RotationMatrix* rotation = new G4RotationMatrix(*old_rotation);
  rotation->rotateX(rotation_xyz.x());
  rotation->rotateY(rotation_xyz.y());
  rotation->rotateZ(rotation_xyz.z());

  // Print verbose
  if (fVerboseLevel > 0)
    G4cout << "Setting rotation in mother volume "
           << "from " << *old_rotation << " "
           << "to " << *rotation << " for " << name << G4endl;

  // Set position for volume
  physical_volume->SetRotation(rotation);

  // Delete old rotation matrix
  delete old_rotation;

  // Reoptimize geometry
  G4RunManager* run_manager = G4RunManager::GetRunManager();
  run_manager->GeometryHasBeenModified();
}


void comptonDetectorConstruction::PrintGDMLWarning() const
{
    G4cout << G4endl;
    G4cout << "compton: Note: GDML file validation can cause many warnings." << G4endl;
    G4cout << "compton: Some can be safely ignore. Here are some guidelines:" << G4endl;
    G4cout << "compton: - 'ID attribute is referenced but was never declared'" << G4endl;
    G4cout << "compton:   If the attribute starts with G4_ it is likely defined" << G4endl;
    G4cout << "compton:   in the NIST materials database and declared later." << G4endl;
    G4cout << "compton: - 'attribute phi is not declared for element direction'" << G4endl;
    G4cout << "compton:   Replication along the phi direction is not supported" << G4endl;
    G4cout << "compton:   by the GDML standard, but it is by geant4." << G4endl;
    G4cout << "compton: - 'no declaration found for element property'" << G4endl;
    G4cout << "compton:   Setting optical properties is not supported" << G4endl;
    G4cout << "compton:   by the GDML standard, but it is by geant4 (e.g. G01)." << G4endl;
    G4cout << G4endl;
}

G4VPhysicalVolume* comptonDetectorConstruction::ParseGDMLFile()
{
    // Clear parser
    fGDMLParser.Clear();

    // Print parsing options
    G4cout << "Reading " << fGDMLFile << G4endl;
    G4cout << "- schema validation " << (fGDMLValidate? "on": "off") << G4endl;
    G4cout << "- overlap check " << (fGDMLOverlapCheck? "on": "off") << G4endl;

    // Print GDML warning when validation
    if (fGDMLValidate) PrintGDMLWarning();

    // Get comptonIO instance before chdir since comptonIO creates root file
    comptonIO* io = comptonIO::GetInstance();

    // Change directory
    char cwd[MAXPATHLEN];
    if (getcwd(cwd,MAXPATHLEN) == nullptr) {
      G4cerr << __FILE__ << " line " << __LINE__ << ": ERROR no current working directory" << G4endl;
      exit(-1);
    }
    if (chdir(fGDMLPath) != 0) {
      G4cerr << __FILE__ << " line " << __LINE__ << ": ERROR cannot change directory" << G4endl;
      exit(-1);
    }

    // Parse GDML file
    fGDMLParser.SetOverlapCheck(fGDMLOverlapCheck);
    // hide output if not validating or checking overlaps
    // https://bugzilla-geant4.kek.jp/show_bug.cgi?id=2358
    if (! fGDMLOverlapCheck && ! fGDMLValidate)
      G4cout.setstate(std::ios_base::failbit);
    fGDMLParser.Read(fGDMLFile,fGDMLValidate);
    G4cout.clear();
    G4VPhysicalVolume* worldvolume = fGDMLParser.GetWorldVolume();

    // Print tolerances
    if (fVerboseLevel > 0) {
      G4cout << "Computed surface tolerance = "
             << G4GeometryTolerance::GetInstance()->GetSurfaceTolerance()/mm
             << " mm" << G4endl;
      G4cout << "Computed angular tolerance = "
             << G4GeometryTolerance::GetInstance()->GetAngularTolerance()/rad
             << " rad" << G4endl;
      G4cout << "Computed radial tolerance = "
             << G4GeometryTolerance::GetInstance()->GetRadialTolerance()/mm
             << " mm" << G4endl;
    }

    // Print overlaps
    if (fGDMLOverlapCheck)
      PrintGeometryTree(worldvolume,0,true,false);

    // Add GDML files to IO
    io->GrabGDMLFiles(fGDMLFile);

    // Change directory back
    if (chdir(cwd) != 0) {
      G4cerr << __FILE__ << " line " << __LINE__ << ": ERROR cannot change directory" << G4endl;
      exit(-1);
    }

    // Return world volume
    return worldvolume;
}

void comptonDetectorConstruction::PrintAuxiliaryInfo() const
{
  const G4GDMLAuxMapType* auxmap = fGDMLParser.GetAuxMap();
  G4cout << "Found " << auxmap->size()
         << " volume(s) with auxiliary information."
         << G4endl << G4endl;
}

void comptonDetectorConstruction::ParseAuxiliaryTargetInfo()
{
    //====================================================
    // Associate target volumes with beam/target class
    //====================================================

    // FIXME
    // This function is somewhat inefficient since it loops over the full
    // map of auxiliary tags in a nested fashion. If someone can figure out
    // how to improve this, you are welcome to :-)

    // Loop over volumes with auxiliary information
    const G4GDMLAuxMapType* auxmap = fGDMLParser.GetAuxMap();
    for(G4GDMLAuxMapType::const_iterator
        iter  = auxmap->begin();
        iter != auxmap->end(); iter++) {

      // Loop over auxiliary tags for this logical volume
      G4LogicalVolume* logical_volume = (*iter).first;
      for (G4GDMLAuxListType::const_iterator
          vit  = (*iter).second.begin();
          vit != (*iter).second.end(); vit++) {

        // Treat auxiliary type "TargetSystem" only
        if ((*vit).type != "TargetSystem") continue;

        // Target system name
        G4String mother_tag = (*vit).value;

        // Found target mother logical volume
        G4LogicalVolume* mother_logical_volume = logical_volume;
        if (fVerboseLevel > 0)
          G4cout << "Found target system mother logical volume "
                 << mother_logical_volume->GetName() << "." << G4endl;

        // Now find target mother physical volume
        G4VPhysicalVolume* mother_physical_volume = 0;
        std::vector<G4VPhysicalVolume*> list =
            GetPhysicalVolumes(fWorldVolume,mother_logical_volume);
        if (list.size() == 1) {
          mother_physical_volume = list[0];

          // Mutex lock before writing static structures in comptonBeamTarget
          G4AutoLock lock(&comptonDetectorConstructionMutex);
          comptonBeamTarget::AddMotherVolume(mother_physical_volume, mother_tag);

          if (fVerboseLevel > 0)
            G4cout << "Found target mother physical volume "
                   << mother_physical_volume->GetName() << "." << G4endl;
        } else {
          G4cout << "Target mother logical volume does not occur "
                 << "*exactly once as a physical volume." << G4endl;
          exit(-1);
        }

        // Loop over target mother logical volume daughters
        auto n = mother_logical_volume->GetNoDaughters();
        for (decltype(n) i = 0; i < n; i++) {

          // Get daughter physical and logical volumes
          G4VPhysicalVolume* target_physical_volume = mother_logical_volume->GetDaughter(i);
          G4LogicalVolume* target_logical_volume = target_physical_volume->GetLogicalVolume();

          // Target volume must contain "Target" auxiliary tag as well
          //
          // TODO Seems like this shouldn't require an iteration over a map,
          // of all things, but I coulnd't get auxmap[target_logical_volume]
          // to work due to (unhelpful) compiler errors, probably related to
          // the use of the typedef instead of actual map. Something like a
          // for (G4GDMLAuxListType::const_iterator vit2 =
          //   auxmap[target_logical_volume].begin(); etc
          for(G4GDMLAuxMapType::const_iterator
              iter2  = auxmap->begin();
              iter2 != auxmap->end(); iter2++) {

            // Only the target logical volume is of interest
            if ((*iter2).first != target_logical_volume) continue;

            for (G4GDMLAuxListType::const_iterator
                 vit2  = (*iter2).second.begin();
                 vit2 != (*iter2).second.end(); vit2++) {

              // If the logical volume is tagged as "TargetSamplingVolume"
              if ((*vit2).type != "TargetSamplingVolume") continue;

              // Target system name
              G4String target_tag = (*vit2).value;

              // Add target volume
              G4cout << "Adding target sampling volume "
                     << target_logical_volume->GetName() << "." << G4endl;
              comptonBeamTarget::AddTargetVolume(target_physical_volume, target_tag);

            } // loop over auxiliary tags in volume to find "TargetSamplingVolume"

          } // loop over volumes with auxiliary tags to find "TargetSamplingVolume"

        } // loop over daughter volumes in target system

      } // loop over auxiliary tags in volume to find "TargetSystem"

    } // loop over volumes with auxiliary tags to find "TargetSystem"

    comptonBeamTarget::UpdateInfo();
}

void comptonDetectorConstruction::ParseAuxiliaryUserLimits()
{
  const G4GDMLAuxMapType* auxmap = fGDMLParser.GetAuxMap();
  for(G4GDMLAuxMapType::const_iterator
      iter  = auxmap->begin();
      iter != auxmap->end(); iter++) {

    if (fVerboseLevel > 0)
      G4cout << "Volume " << ((*iter).first)->GetName()
             << " has the following list of auxiliary information: "<< G4endl;

    // Loop over auxiliary tags for this logical volume
    G4LogicalVolume* logical_volume = (*iter).first;
    for (G4GDMLAuxListType::const_iterator
        vit  = (*iter).second.begin();
        vit != (*iter).second.end(); vit++) {

      if (fVerboseLevel > 0)
        G4cout << "--> Type: " << (*vit).type
	       << " Value: "   << (*vit).value << std::endl;

      // Skip if not starting with "User"
      #if G4VERSION_NUMBER < 1100
      if (! (*vit).type.contains("User")) continue;
      #else
      if (! G4StrUtil::contains((*vit).type, "User")) continue;
      #endif

      // Set user limits
      SetUserLimits(logical_volume, (*vit).type, (*vit).value);
    }
  }

  if (fVerboseLevel > 0)
      G4cout << G4endl << G4endl;
}

void comptonDetectorConstruction::ParseAuxiliaryVisibilityInfo()
{
  // Loop over volumes with auxiliary information
  const G4GDMLAuxMapType* auxmap = fGDMLParser.GetAuxMap();
  for(G4GDMLAuxMapType::const_iterator
      iter  = auxmap->begin();
      iter != auxmap->end(); iter++) {

    if (fVerboseLevel > 0)
      G4cout << "Volume " << ((*iter).first)->GetName()
             << " has the following list of auxiliary information: "<< G4endl;

    // Loop over auxiliary tags for this logical volume
    for (G4GDMLAuxListType::const_iterator
         vit  = (*iter).second.begin();
         vit != (*iter).second.end(); vit++) {

      if (fVerboseLevel > 0)
        G4cout << "--> Type: " << (*vit).type
	       << " Value: "   << (*vit).value << std::endl;

      // Visibility = true|false|wireframe
      if ((*vit).type == "Visibility") {
        G4Colour colour(1.0,1.0,1.0);
        const G4VisAttributes* visAttribute_old = ((*iter).first)->GetVisAttributes();
        if (visAttribute_old != nullptr)
          colour = visAttribute_old->GetColour();
        G4VisAttributes visAttribute_new(colour);
        if ((*vit).value == "true")
          visAttribute_new.SetVisibility(true);
        if ((*vit).value == "false")
          visAttribute_new.SetVisibility(false);
        if ((*vit).value == "wireframe")
          visAttribute_new.SetForceWireframe(false);

        ((*iter).first)->SetVisAttributes(visAttribute_new);
      }

      // Color = name
      if ((*vit).type == "Color") {
        G4Colour colour(1.0,1.0,1.0);
        if (G4Colour::GetColour((*vit).value, colour)) {

          if (fVerboseLevel > 0)
            G4cout << "Setting color to " << (*vit).value << "." << G4endl;

          G4VisAttributes visAttribute(colour);
          ((*iter).first)->SetVisAttributes(visAttribute);

        } else {

          if (fVerboseLevel > 0)
            G4cout << "Colour " << (*vit).value << " is not known." << G4endl;

        }
      }

      // Alpha = float between 0 and 1
      if ((*vit).type == "Alpha") {
        G4Colour colour(1.0,1.0,1.0);
        const G4VisAttributes* visAttribute_old = ((*iter).first)->GetVisAttributes();

        if (visAttribute_old != nullptr)
          colour = visAttribute_old->GetColour();

        G4Colour colour_new(
            colour.GetRed(),
            colour.GetGreen(),
            colour.GetBlue(),
            std::atof((*vit).value.c_str()));
        G4VisAttributes visAttribute_new(colour_new);
        ((*iter).first)->SetVisAttributes(visAttribute_new);
      }
    }
  }
  if (fVerboseLevel > 0)
      G4cout << G4endl << G4endl;


  // Set the world volume to wireframe
  G4VisAttributes* motherVisAtt = new G4VisAttributes(G4Colour(1.0,1.0,1.0));
  motherVisAtt->SetForceWireframe(true);
  fWorldVolume->GetLogicalVolume()->SetVisAttributes(motherVisAtt);
}

void comptonDetectorConstruction::ParseAuxiliarySensDetInfo()
{
  if (fVerboseLevel > 0)
      G4cout << "Beginning sensitive detector assignment" << G4endl;

  // Duplication map
  std::map<int, std::pair<G4String, G4LogicalVolume*>> detnomap;

  // Loop over all volumes with auxiliary tags
  const G4GDMLAuxMapType* auxmap = fGDMLParser.GetAuxMap();
  for (G4GDMLAuxMapType::const_iterator iter  = auxmap->begin(); iter != auxmap->end(); iter++) {

      G4LogicalVolume* myvol = (*iter).first;
      G4GDMLAuxListType list = (*iter).second;

      if (fVerboseLevel > 0)
        G4cout << "Volume " << myvol->GetName() << G4endl;

      comptonGenericDetector* comptonsd = 0;

      // Find first aux list entry with type SensDet
      auto it_sensdet = NextAuxWithType(list.begin(), list.end(), "SensDet");
      if (it_sensdet != list.end()) {

        G4String sens_det = it_sensdet->value;

        // Find first aux list entry with type DetNo
        auto it_detno = NextAuxWithType(list.begin(), list.end(), "DetNo");
        if (it_detno != list.end()) {

          int det_no = atoi(it_detno->value.data());
          bool enabled = (det_no > 0)? false : true;
          det_no = std::abs(det_no);

          // Construct detector name
          std::stringstream det_name_ss;
          //det_name_ss << "compton/det_" << det_no<<myvol->GetName();
          det_name_ss << "compton/det_" <<myvol->GetName(); //<<"_"<<det_no;
          std::string det_name = det_name_ss.str();

          // Check for duplication when not a shared detector number
          if (detnomap.count(det_no) != 0 && detnomap[det_no].first != sens_det) {
            std::string sens_det2 = detnomap[det_no].first;
            G4LogicalVolume* myvol2 = detnomap[det_no].second;
            G4cerr << "compton: Detector number " << det_no << " (" << sens_det  << ") " << myvol->GetName()  << G4endl;
            G4cerr << "compton: already used by " << det_no << " (" << sens_det2 << ") " << myvol2->GetName() << G4endl;
          }

          // Try to find sensitive detector
          G4SDManager* SDman = G4SDManager::GetSDMpointer();
          G4VSensitiveDetector* sd = SDman->FindSensitiveDetector(det_name, (fVerboseLevel > 0));
          // and cast into compton sensitive detector
          comptonsd = dynamic_cast<comptonGenericDetector*>(sd);

          // No such detector yet
          if (comptonsd == 0) {

            if (fVerboseLevel > 0)
              G4cout << "  Creating sensitive detector "
                     << "for volume " << myvol->GetName()
                     <<  G4endl;

            comptonsd = new comptonGenericDetector(det_name, det_no);
            comptonsd->SetEnabled(enabled);

            // Register detector with SD manager
            SDman->AddNewDetector(comptonsd);
            detnomap[det_no] = std::make_pair(sens_det, myvol);

            // Register detector with comptonIO
            comptonIO* io = comptonIO::GetInstance();
            io->RegisterDetector(myvol->GetName(), sens_det, det_no);

          }

          // Register detector with this volume
          myvol->SetSensitiveDetector(comptonsd);

        } // end of if aux tag with type DetNo

      } // end of if aux tag with type SensDet


      // Find aux list entries with type DetType
      for (auto it_dettype  = NextAuxWithType(list.begin(), list.end(), "DetType");
                it_dettype != list.end();
                it_dettype  = NextAuxWithType(++it_dettype, list.end(), "DetType")) {

        // Set detector type
        if (comptonsd != nullptr) comptonsd->SetDetectorType(it_dettype->value);

        // Print detector type
        if (fVerboseLevel > 0)
          if (comptonsd != nullptr) comptonsd->PrintDetectorType();

      }

  } // end of loop over volumes

  if (fVerboseLevel > 0)
    G4cout << "Completed sensitive detector assignment" << G4endl;
}

G4VPhysicalVolume* comptonDetectorConstruction::Construct()
{
  // Parse GDML file
  fWorldVolume = ParseGDMLFile();

  // Parse auxiliary info
  PrintAuxiliaryInfo();
  ParseAuxiliaryTargetInfo();
  ParseAuxiliaryUserLimits();
  ParseAuxiliaryVisibilityInfo();

  // Set copy number of geometry tree
  UpdateCopyNo(fWorldVolume,1);

  // Set kryptonite user limits
  InitKryptoniteMaterials();
  SetKryptoniteUserLimits(fWorldVolume);

  return fWorldVolume;
}

void comptonDetectorConstruction::LoadMagneticField()
{
  // Remove existing field and load new field
  delete fGlobalField;
  fGlobalField = new comptonGlobalField();
}

void comptonDetectorConstruction::ConstructSDandField()
{
  // Parse auxiliary info
  ParseAuxiliarySensDetInfo();

  // Load magnetic field
  LoadMagneticField();
}


void comptonDetectorConstruction::SetUserLimits(
    const G4String& set_type,
    const G4String& name,
    const G4String& value_units) const
{
  // Find volume
  G4LogicalVolume* logical_volume = G4LogicalVolumeStore::GetInstance()->GetVolume(name);
  if (logical_volume == nullptr) {
    G4cerr << __FILE__ << " line " << __LINE__ << ": Warning volume " << name << " unknown" << G4endl;
    return;
  }

  // Remove starting "Set" used by commands
  G4String type = set_type;
  if (type.find("Set") == 0) type.erase(0,3);

  if (fVerboseLevel > 0)
    G4cout << "Setting user limit " << type << " for " << name << G4endl;

  // Set user limits
  SetUserLimits(logical_volume, type, value_units);
}

void comptonDetectorConstruction::SetUserLimits(
    G4LogicalVolume* logical_volume,
    const G4String& type,
    const G4String& value_units) const
{
  // Get user limits
  G4UserLimits* userlimits = logical_volume->GetUserLimits();
  if (userlimits == nullptr) {
    userlimits = new G4UserLimits();
    logical_volume->SetUserLimits(userlimits);
  }

  // Set user limits
  SetUserLimits(userlimits, type, value_units);
}

void comptonDetectorConstruction::SetUserLimits(
    G4UserLimits* userlimits,
    const G4String& type,
    const G4String& value_units) const
{
  if (fVerboseLevel > 0)
    G4cout << "Setting user limit " << type << " to " << value_units << G4endl;

  // Resolve units in value_units
  G4String value_space_units = value_units;
  std::replace(value_space_units.begin(), value_space_units.end(), '*', ' ');
  G4double value = G4UIcmdWithADoubleAndUnit::GetNewDoubleValue(value_space_units);

  // Compare with allowed types while ignoring case
  auto icompare = [](const G4String& lhs, const G4String& rhs) {
    #if G4VERSION_NUMBER < 1100
      return lhs.compareTo(rhs, G4String::ignoreCase);
    #else
      return G4StrUtil::icompare(lhs, rhs);
    #endif
  };
  if      (icompare(type, "usermaxallowedstep") == 0)
    userlimits->SetMaxAllowedStep(value);
  else if (icompare(type, "usermaxtracklength") == 0)
    userlimits->SetUserMaxTrackLength(value);
  else if (icompare(type, "usermaxtime") == 0)
    userlimits->SetUserMaxTime(value);
  else if (icompare(type, "userminekine") == 0)
    userlimits->SetUserMinEkine(value);
  else if (icompare(type, "userminrange") == 0)
    userlimits->SetUserMinRange(value);
  else
    G4cerr << __FILE__ << " line " << __LINE__ << ": Warning user type " << type << " unknown" << G4endl;
}

void comptonDetectorConstruction::ReloadGeometry(const G4String gdmlfile)
{
  // Set new geometry
  SetGDMLFile(gdmlfile);

  // Trigger Construct and ConstructSDandField
  G4RunManager::GetRunManager()->ReinitializeGeometry(true);
}

G4int comptonDetectorConstruction::UpdateCopyNo(G4VPhysicalVolume* aVolume,G4int index)
{
  //if (aVolume->GetLogicalVolume()->GetNoDaughters()==0 ){
      aVolume->SetCopyNo(index);
      G4Material* material;
      G4VisAttributes* kryptoVisAtt= new G4VisAttributes(G4Colour(0.7,0.0,0.0));
      //set user limits for Kryptonite materials. When tracks are killed inside Kryptonite materials, energy will be properly deposited
      material = aVolume->GetLogicalVolume()->GetMaterial();
      if (material->GetName() == "Kryptonite") {
	aVolume->GetLogicalVolume()->SetUserLimits( new G4UserLimits(0.0, 0.0, 0.0, DBL_MAX, DBL_MAX) );
	aVolume->GetLogicalVolume()->SetVisAttributes(kryptoVisAtt);
      }
      index++;
      //}else {
    auto n = aVolume->GetLogicalVolume()->GetNoDaughters();
    for(decltype(n) i=0;i<n;i++){
      index = UpdateCopyNo(aVolume->GetLogicalVolume()->GetDaughter(i),index);
    }
    //}

  return index;
}

void comptonDetectorConstruction::PrintElements() {
  G4cout << G4endl << "Element table: " << G4endl << G4endl;
  G4cout << *(G4Element::GetElementTable()) << G4endl;
}

void comptonDetectorConstruction::PrintMaterials() {
  G4cout << G4endl << "Material table: " << G4endl << G4endl;
  G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

std::vector<G4VPhysicalVolume*> comptonDetectorConstruction::GetPhysicalVolumes(
    G4VPhysicalVolume* physical_volume,
    const G4LogicalVolume* logical_volume)
{
  // Create list of results
  std::vector<G4VPhysicalVolume*> list;

  // Store as result if the logical volume name agrees
  if (physical_volume->GetLogicalVolume() == logical_volume) {
    list.push_back(physical_volume);
  }

  // Descend down the tree
  auto n = physical_volume->GetLogicalVolume()->GetNoDaughters();
  for (decltype(n) i = 0; i < n; i++)
  {
    // Get results for daughter volumes
    std::vector<G4VPhysicalVolume*> daughter_list =
        GetPhysicalVolumes(physical_volume->GetLogicalVolume()->GetDaughter(i),logical_volume);
    // Add to the list of results
    list.insert(list.end(),daughter_list.begin(),daughter_list.end());
  }

  return list;
}

void comptonDetectorConstruction::PrintGeometryTree(
    G4VPhysicalVolume* aVolume,
    G4int depth,
    G4bool surfchk,
    G4bool print)
{
  // Null volume
  if (aVolume == 0) aVolume = fWorldVolume;

  // Print spaces
  if (print) {
    for (int isp = 0; isp < depth; isp++) { G4cout << "  "; }
  }
  // Print name
  if (print) {
    G4cout << aVolume->GetName() << "[" << aVolume->GetCopyNo() << "] "
           << aVolume->GetLogicalVolume()->GetName() << " "
           << aVolume->GetLogicalVolume()->GetNoDaughters() << " "
           << aVolume->GetLogicalVolume()->GetMaterial()->GetName() << " "
           << G4BestUnit(aVolume->GetLogicalVolume()->GetMass(true),"Mass");
  }
  // Print sensitive detector
  G4VSensitiveDetector* sd = aVolume->GetLogicalVolume()->GetSensitiveDetector();
  if (print && (sd != nullptr))
  {
    comptonGenericDetector* comptonsd = dynamic_cast<comptonGenericDetector*>(sd);
    G4cout << " [" << comptonsd->GetDetNo() << "]";
  }
  if (print) {
    G4cout << G4endl;
  }

  // Check overlapping volumes (tolerance of 1 mm)
  if (surfchk) aVolume->CheckOverlaps(1000,1.0*mm,false);

  // Descend down the tree
  auto n = aVolume->GetLogicalVolume()->GetNoDaughters();
  for (decltype(n) i = 0; i < n; i++) {
    PrintGeometryTree(aVolume->GetLogicalVolume()->GetDaughter(i),depth+1,surfchk,print);
  }
}
