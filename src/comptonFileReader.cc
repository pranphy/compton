#include "comptonFileReader.hh"

comptonFileReader::comptonFileReader(G4String filename, G4int skip, G4int debuglevel)
: fDebugLevel(debuglevel)
{
  fInputFile.open(filename.data());

  if (fDebugLevel > 0)
    G4cout << "Opening " << filename << G4endl;

  while (skip-- > 0) {
    std::string line;
    std::getline(fInputFile, line);
    if (fDebugLevel > 1)
      G4cout << "Skipping: " << line << G4endl;
  }

  if (fDebugLevel > 0)
    G4cout << filename << " opened." << G4endl;
}

comptonFileReader::~comptonFileReader()
{
  fInputFile.close();
}

comptonFileEvent comptonFileReader::GetAnEvent()
{
  if (fEventList.size() == 0)
  {
    for (int i = 0; i < 100 && !fInputFile.eof(); i++)
    {
      G4double vx, vy, vz, px, py, pz, w;
      while ((fInputFile >> vx >> vy >> vz >> px >> py >> pz >> w)) {
        fEventList.push_back(comptonFileEvent(G4ThreeVector(vx,vy,vz),G4ThreeVector(px,py,pz),w));
      }
      fInputFile.clear();
      std::string line;
      std::getline(fInputFile, line);
      if (fDebugLevel > 1) {
        if (!fInputFile.eof()) G4cout << "Skipping: " << line << G4endl;
        else G4cout << "EOF reached." << G4endl;
      }
    }
  }
  comptonFileEvent event = fEventList.front();
  fEventList.pop_front();
  return event;
}
