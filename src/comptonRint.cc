#include "comptonRint.hh"

// Standard C and C++ headers
#include <iostream>
#include <cstdlib>

// ROOT headers
#include <TROOT.h>

// Global pointers
comptonRint* gRemollRint = NULL;

// Pointer to self
comptonRint* comptonRint::fExists = NULL;


//--------------------------------------------------------------------------
comptonRint::comptonRint (const char* appClassName, int* argc, char** argv,
                        void* options, int numOptions, bool noLogo)
: TRint (appClassName, argc, argv, options, numOptions, noLogo)
{
  gRemollRint = this;

  // re-root command prompt
  SetPrompt("compoot [%d] ");

  // Pointer to self
  fExists = this;
}

//---------------------------------------------------------------------------
comptonRint::~comptonRint()
{
  // Reset point to self
  if (fExists == this)
    fExists = NULL;
}
