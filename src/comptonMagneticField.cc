#include "comptonMagneticField.hh"
#include "G4UImanager.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include "comptonSearchPath.hh"

#include <iostream>
#include <fstream>

#include <assert.h>
#include <math.h>

comptonMagneticField::comptonMagneticField(const G4String& filename)
    : fName(filename),
    fFilename(filename),
    fN{0,0,0},
    fOffset{0,0,0},
    fUnit{m,m,m},
    fMin{0.0},
    fMax{0.0},
    fStep{0.0},
    fFileMin{0.0},
    fFileMax{0.0},
    fFieldValue(0.0),
    fFieldScale(1.0),
    fRefCurrent(0.0),
    fInterpolationType(kLinear)
{
    std::ifstream inputfile;

    inputfile.open(fFilename.data());

    if (!inputfile.good() ){
        G4cerr << "Error " << __FILE__ << " line " << __LINE__ << G4endl << G4endl << G4endl << G4endl << G4endl
            << ": File " << fFilename << " could not open.  Aborting" << G4endl;
        exit(1);
    }

    std::string inputline;

    for (size_t cidx : {kX, kY, kZ} ) {
        getline(inputfile,inputline);
        if (std::istringstream(inputline) >> fN[cidx] >> fMin[cidx] >> fMax[cidx] >> fOffset[cidx]) {
            fMin[cidx] *= fUnit[cidx];
            fMax[cidx] *= fUnit[cidx];
            fOffset[cidx] *= fUnit[cidx];
        } else {
            G4cerr << "Error " << __FILE__ << " line " << __LINE__
                << ": File " << fFilename << " contains unreadable header.  Aborting" << G4endl;
            exit(1);
        }
    }


    getline(inputfile,inputline);
    if (std::istringstream(inputline) >> fRefCurrent) {
        G4cout << "RefCurrent = " << fRefCurrent << G4endl;
    } else {
        G4cerr << "Error " << __FILE__ << " line " << __LINE__
            << ": File " << fFilename << " contains unreadable header.  Aborting" << G4endl;
        exit(1);
    }
    // Sanity check on header data

    for( auto cidx : {kX, kY, kZ} ){
        if( !( fMin[cidx] < fMax[cidx] &&  fN[cidx] > 0 )){
            G4cerr << "Error " << __FILE__ << " line " << __LINE__ << ": File " << fFilename << "sanity check failed; Aborting" << G4endl;
            exit(1);
        }
    }


    // Dynamically size 3D vectors to what we need to store the file
    for (size_t cidx : {kX,kY,kZ}) {
        // Set up storage space for table
        fBFieldData[cidx].clear();
        fBFieldData[cidx].resize(fN[kX], std::vector<std::vector<G4double>>(fN[kY], std::vector<G4double>(fN[kZ], 0.0)));
    }

    G4int nlines = 0;
    for (size_t zidx = 0; zidx < fN[kZ]; zidx++) {
        for (size_t pidx = 0; pidx < fN[kY]; pidx++) {
            for (size_t ridx = 0; ridx < fN[kX]; ridx++) {
                getline(inputfile,inputline);

                // Read in field values and assign units
                G4double x, y, z, bx, by, bz;
                if (std::istringstream(inputline) >> x >> y >> z >> bx >> by >> bz) {
                    nlines++;
                } else {
                    G4cerr << "Error " << __FILE__ << " line " << __LINE__
                        << ": File " << fFilename << " contains invalid data.  Aborting" << G4endl;
                    exit(1);
                }

                fBFieldData[kX][ridx][pidx][zidx] = bx*tesla;
                fBFieldData[kY][ridx][pidx][zidx] = by*tesla;
                fBFieldData[kZ][ridx][pidx][zidx] = bz*tesla;
            }
        }
    }
    std::cout<<" Read "<<nlines<<" from the file "<<std::endl;

    G4cout << "... done reading successfully" << G4endl << G4endl;
    std::cout<<" By my calculation"<<std::endl;
    std::cout<<" x off "<<fOffset[kX]<<" y off "<<fOffset[kY]<< " z off "<<fOffset[kZ]<<std::endl;
    std::cout<<" x from "<<fMin[kX]+fOffset[kX]<<" To "<<fMax[kX] + fOffset[kX]<<std::endl;
    std::cout<<" y from "<<fMin[kY]+fOffset[kY]<<" To "<<fMax[kY] + fOffset[kY]<<std::endl;
    std::cout<<" z from "<<fMin[kZ]+fOffset[kZ]<<" To "<<fMax[kZ] + fOffset[kZ]<<std::endl;
}

void comptonMagneticField::GetFieldValue(const G4double point[4], G4double *field ) const
{
    // set to zero
    field[0] = 0.0;
    field[1] = 0.0;
    field[2] = 0.0;
    // add values
    AddFieldValue(point, field);
}

void comptonMagneticField::AddFieldValue(const G4double point[4], G4double *field ) const
{
    // Check the bounding box
    if (! IsInBoundingBox(point)) return;
    field[kX] += fBFieldData[kX][0][0][0];
    field[kY] += fBFieldData[kY][0][0][0];
    field[kZ] += fBFieldData[kZ][0][0][0];
    //field[kX] += 1.5*tesla;
    //field[kY] += 0;
    //field[kZ] += 0;
}
