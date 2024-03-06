#include "comptonRun.hh"

#include "comptonRunData.hh"

comptonRunData* comptonRun::fRunData = 0;

comptonRunData* comptonRun::GetRunData()
{
  if (fRunData == nullptr) {
    fRunData = new comptonRunData();
    fRunData->Init();
  }
  return fRunData;
}
