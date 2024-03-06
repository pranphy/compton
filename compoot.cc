// ROOT headers
#include <TROOT.h>
#include <TRint.h>
#include <TSystem.h>
#include <TString.h>

#include "comptonRint.hh"

int main(int argc, char** argv)
{
  // Start Root command prompt
  comptonRint* rint = new comptonRint("compton ROOT Analyzer", &argc, argv);

  // Setup include path
  gROOT->ProcessLine(".include include");
  rint->Run();
  delete rint;
}
