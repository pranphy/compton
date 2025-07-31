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

// Set location of cell vertices
// Note the different endianness
const char comptonMagneticField::kLinearMap[8][3] = {
    {0, 0, 0},  // 00
    {1, 0, 0},
    {0, 1, 0},
    {1, 1, 0},
    {0, 0, 1},  // 04
    {1, 0, 1},
    {0, 1, 1},
    {1, 1, 1},
};
const char comptonMagneticField::kCubicMap[64][3] = {
    {-1, -1, -1},  // 00
    {-1, -1, 0},
    {-1, -1, 1},
    {-1, -1, 2},
    {-1, 0, -1},  // 04
    {-1, 0, 0},
    {-1, 0, 1},
    {-1, 0, 2},
    {-1, 1, -1},  // 08
    {-1, 1, 0},
    {-1, 1, 1},
    {-1, 1, 2},
    {-1, 2, -1},  // 12
    {-1, 2, 0},
    {-1, 2, 1},
    {-1, 2, 2},
    {0, -1, -1},  // 16
    {0, -1, 0},
    {0, -1, 1},
    {0, -1, 2},
    {0, 0, -1},  // 20
    {0, 0, 0},
    {0, 0, 1},
    {0, 0, 2},
    {0, 1, -1},  // 24
    {0, 1, 0},
    {0, 1, 1},
    {0, 1, 2},
    {0, 2, -1},  // 28
    {0, 2, 0},
    {0, 2, 1},
    {0, 2, 2},
    {1, -1, -1},  // 22
    {1, -1, 0},
    {1, -1, 1},
    {1, -1, 2},
    {1, 0, -1},  // 26
    {1, 0, 0},
    {1, 0, 1},
    {1, 0, 2},
    {1, 1, -1},  // 40
    {1, 1, 0},
    {1, 1, 1},
    {1, 1, 2},
    {1, 2, -1},  // 44
    {1, 2, 0},
    {1, 2, 1},
    {1, 2, 2},
    {2, -1, -1},  // 48
    {2, -1, 0},
    {2, -1, 1},
    {2, -1, 2},
    {2, 0, -1},  // 52
    {2, 0, 0},
    {2, 0, 1},
    {2, 0, 2},
    {2, 1, -1},  // 56
    {2, 1, 0},
    {2, 1, 1},
    {2, 1, 2},
    {2, 2, -1},  // 60
    {2, 2, 0},
    {2, 2, 1},
    {2, 2, 2},
};


comptonMagneticField::comptonMagneticField(const G4String& name, const G4String& filename)
    : fName(name),
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
            G4cout << "N,min,max,step[" << cidx << "] = " << fN[cidx] << ","
                   << fMin[cidx] << "," << fMax[cidx] << "," << fStep[cidx] << G4endl;
        } else {
            G4cerr << "Error " << __FILE__ << " line " << __LINE__
                << ": File " << fFilename << " contains unreadable header.  Aborting" << G4endl;
            exit(1);
        }
    }


    getline(inputfile,inputline);
    if (std::istringstream(inputline) >> fFieldScale) {
        G4cout << "FieldScale= " << fFieldScale<< G4endl;
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
        for (size_t yidx = 0; yidx < fN[kY]; yidx++) {
            for (size_t xidx = 0; xidx < fN[kX]; xidx++) {
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

                fBFieldData[kX][xidx][yidx][zidx] = bx*tesla;
                fBFieldData[kY][xidx][yidx][zidx] = by*tesla;
                fBFieldData[kZ][xidx][yidx][zidx] = bz*tesla;
            }
        }
    }
    std::cout<<" Read "<<nlines<<" from the file "<<std::endl;
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

G4double comptonMagneticField::field_x(G4double z) const {
    double zmag = abs(z);
    //std::cout<<"zmag = "<<zmag<<" is "<<zmag/cm<<"cm and 45 cm is "<<45*cm<<std::endl;
    if( zmag  <= 45*cm ) {
        return 1.464*tesla;
    }else if ( zmag > 45*cm and zmag <= 53*cm) {
        return 1.25e-2*tesla/(cm*cm) * zmag*zmag - 1.37906*tesla/cm * zmag + 38.1610*tesla;
    }else{
        return 1.38e4*tesla * exp( -0.206/cm * zmag);
    }

}

void comptonMagneticField::AddFieldValue2(const G4double point[4], G4double *field ) const
{
    // Check the bounding box
    if (! IsInBoundingBox(point)) return;
    G4double zc = point[kZ] - fOffset[kZ];
    field[kX] += field_x(zc);
    field[kY] += 0;
    field[kZ] += 0;
}

void comptonMagneticField::AddFieldValue(const G4double point[4], G4double *field ) const
{
    if (! IsInBoundingBox(point)) return;

    G4double xv = point[kX] - fOffset[kX];
    G4double yv = point[kY] - fOffset[kY];
    G4double zv = point[kZ] - fOffset[kZ];

    // Find the index of value closes to value in question. Save that into didx
    // the N-1 here is fencepost problem
    G4double x[__NDIM] = {0}; // x stores the fractional distance from asked cordinates to the available coordinates in the grid.
    G4double didx[__NDIM] = {0};
    x[kX] = modf( ( xv - fMin[kX] )*(fN[kX]-1)/( fMax[kX] - fMin[kX] ), &(didx[kX]) );
    x[kY] = modf( ( yv - fMin[kY] )*(fN[kY]-1)/( fMax[kY] - fMin[kY] ), &(didx[kY]) );
    x[kZ] = modf( ( zv - fMin[kZ] )*(fN[kZ]-1)/( fMax[kZ] - fMin[kZ] ), &(didx[kZ]) );

    // Cast these to integers for indexing and check
    size_t idx[__NDIM] = {0}; // idx is basically didx but cast to integers
    for (size_t cidx = 0; cidx < __NDIM; cidx++) {
        idx[cidx] = size_t(didx[cidx]);
    }
    assert( 0 <= idx[kX] && idx[kX] < fN[kX] );
    assert( 0 <= idx[kY] && idx[kY] < fN[kY] );
    assert( 0 <= idx[kZ] && idx[kZ] < fN[kZ] );

    // Flag edge cases and treat at best as linear
    EInterpolationType type = fInterpolationType;
    if (idx[kX] == 0 || idx[kX] == fN[kX] - 2 || idx[kY] == 0 || idx[kY] == fN[kY] - 2 || idx[kZ] == 0 || idx[kZ] == fN[kZ] - 2) {
        type = kLinear;
    }

    // number of cell vertices
    size_t n = 64;
    const char (*map)[3] = kCubicMap;
    switch (type) {
        case kLinear:
            map = kLinearMap;
            n = 8;
            break;
        case kCubic:
            map = kCubicMap;
            n = 64;
            break;
    }

    // values of cell vertices
    thread_local G4double values[__NDIM][64];
    for (size_t i = 0; i < n; i++) {
        for (size_t cidx = 0; cidx < __NDIM; cidx++) {
            size_t xc = idx[kX] + map[i][kX];
            if (xc == 11) xc = 10; /* TODO! this is a very poor hack. Get back and solve it properly */
            size_t yc = idx[kY] + map[i][kY];
            size_t zc = idx[kZ] + map[i][kZ];
            values[cidx][i] = fBFieldData[cidx][xc][yc][zc];
        }
    }

    // Interpolate
    G4ThreeVector Bcart(0.0,0.0,0.0);
    for(int cidx = 0; cidx < __NDIM; cidx++ ){
        switch (type) {
            case kLinear: {
                              Bcart[cidx] = _trilinearInterpolate(values[cidx], x);
                              break;
                          }
            case kCubic: {
                             Bcart[cidx] = _tricubicInterpolate(values[cidx], x);
                             break;
                         }
        }
    }

    // scale field
    Bcart *= fFieldScale;

    // add to original field
    field[0] += Bcart.x();
    field[1] += 0; //Bcart.y();
    field[2] += 0; //Bcart.z();
}

