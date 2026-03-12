#include "comptonMagneticField.hh"
#include "G4UImanager.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

#include "comptonSearchPath.hh"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cassert>

// Set location of cell vertices
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
    {1, -1, -1},  // 32
    {1, -1, 0},
    {1, -1, 1},
    {1, -1, 2},
    {1, 0, -1},  // 36
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
    fN{0, 0, 0},
    fOffset{0., 0., 0.},
    fUnit{m, m, m},
    fMin{0.0, 0.0, 0.0},
    fMax{0.0, 0.0, 0.0},
    fStep{0.0, 0.0, 0.0},
    fFileMin{0.0, 0.0, 0.0},
    fFileMax{0.0, 0.0, 0.0},
    fXMapOffset(0.0),
    fZMapOffset(0.0),
    fYMapOffset(0.0),
    fFieldValue(0.0),
    fFieldScale(1.0),
    fGlobalScale(1.0),
    fRefCurrent(0.0),
    fInterpolationType(kLinear)
{
    std::ifstream inputfile(fFilename.data());
    if (!inputfile.good()) {
        G4cerr << "Error " << __FILE__ << " line " << __LINE__
            << ": File " << fFilename << " could not open. Aborting." << G4endl;
        std::exit(1);
    }

    std::string inputline;

    // Read header: N, min, max, offset for each axis
    for (size_t cidx : {kX, kY, kZ}) {
        if (!std::getline(inputfile, inputline)) {
            G4cerr << "Error " << __FILE__ << " line " << __LINE__
                << ": Unexpected end of header in " << fFilename << G4endl;
            std::exit(1);
        }

        std::istringstream iss(inputline);
        if (iss >> fN[cidx] >> fFileMin[cidx] >> fFileMax[cidx] >> fOffset[cidx]) {
            fMin[cidx]    = fFileMin[cidx] * fUnit[cidx];
            fMax[cidx]    = fFileMax[cidx] * fUnit[cidx];
            fOffset[cidx] = fOffset[cidx]  * fUnit[cidx];

            if (fN[cidx] > 1) {
                fStep[cidx] = (fMax[cidx] - fMin[cidx]) / (fN[cidx] - 1);
            } else {
                fStep[cidx] = 0.0;
            }

            G4cout << "Axis " << cidx
                << " N=" << fN[cidx]
                << " min=" << fMin[cidx]
                << " max=" << fMax[cidx]
                << " step=" << fStep[cidx]
                << G4endl;
        } else {
            G4cerr << "Error " << __FILE__ << " line " << __LINE__
                << ": File " << fFilename << " contains unreadable header. Aborting." << G4endl;
            std::exit(1);
        }
    }

    // Field scale
    if (!std::getline(inputfile, inputline)) {
        G4cerr << "Error " << __FILE__ << " line " << __LINE__
            << ": Missing field scale line in " << fFilename << G4endl;
        std::exit(1);
    }
    {
        std::istringstream iss(inputline);
        if (!(iss >> fFieldScale)) {
            G4cerr << "Error " << __FILE__ << " line " << __LINE__
                << ": File " << fFilename << " contains unreadable field scale. Aborting."
                << G4endl;
            std::exit(1);
        }
        G4cout << "FieldScale = " << fFieldScale << G4endl;
    }

    // Sanity check on header data
    for (auto cidx : {kX, kY, kZ}) {
        if (!(fMin[cidx] < fMax[cidx] && fN[cidx] > 1)) {
            G4cerr << "Error " << __FILE__ << " line " << __LINE__
                << ": File " << fFilename << " sanity check failed. Aborting."
                << G4endl;
            std::exit(1);
        }
    }

    // Allocate storage
    for (size_t cidx : {kX, kY, kZ}) {
        fBFieldData[cidx].clear();
        fBFieldData[cidx].resize( fN[kX], std::vector<std::vector<G4double>>( fN[kY], std::vector<G4double>(fN[kZ], 0.0)));
    }

    // Read field data
    G4int nlines = 0;
    for (size_t zidx = 0; zidx < fN[kZ]; ++zidx) {
        for (size_t yidx = 0; yidx < fN[kY]; ++yidx) {
            for (size_t xidx = 0; xidx < fN[kX]; ++xidx) {
                if (!std::getline(inputfile, inputline)) {
                    G4cerr << "Error " << __FILE__ << " line " << __LINE__
                        << ": Unexpected end of data in " << fFilename << G4endl;
                    std::exit(1);
                }

                G4double x, y, z, bx, by, bz;
                std::istringstream iss(inputline);
                if (iss >> x >> y >> z >> bx >> by >> bz) {
                    ++nlines;
                    fBFieldData[kX][xidx][yidx][zidx] = bx * tesla;
                    fBFieldData[kY][xidx][yidx][zidx] = by * tesla;
                    fBFieldData[kZ][xidx][yidx][zidx] = bz * tesla;
                } else {
                    G4cerr << "Error " << __FILE__ << " line " << __LINE__
                        << ": File " << fFilename << " contains invalid data. Aborting." << G4endl;
                    std::exit(1);
                }
            }
        }
    }

    G4cout << "Read " << nlines << " field lines from file " << fFilename << G4endl;
}

void comptonMagneticField::GetFieldValue(const G4double point[4], G4double* field) const
{
    field[0] = 0.0;
    field[1] = 0.0;
    field[2] = 0.0;

    AddFieldValue(point, field);
}

G4double comptonMagneticField::field_x(G4double z) const {
    const double zmag = std::abs(z);

    if (zmag <= 45 * cm) {
        return 1.464 * tesla;
    } else if (zmag > 45 * cm && zmag <= 53 * cm) {
        return 1.25e-2 * tesla / (cm * cm) * zmag * zmag - 1.37906 * tesla / cm * zmag + 38.1610 * tesla;
    } else {
        return 1.38e4 * tesla * std::exp(-0.206 / cm * zmag);
    }
}

void comptonMagneticField::AddFieldValue2(const G4double point[4], G4double* field) const
{
    if (!IsInBoundingBox(point)) return;

    G4double zc = point[kZ] - fOffset[kZ];
    field[kX] += field_x(zc);
    field[kY] += 0.0;
    field[kZ] += 0.0;
}

void comptonMagneticField::ComputeIndexAndFraction(G4double coord, size_t axis, size_t& idx, G4double& frac) const
{
    // Map coord to [0, 1) in the grid, then to [0, N-1)
    const G4double u = (coord - fMin[axis]) / (fMax[axis] - fMin[axis]);
    // Clamp to [0, 1 - eps] to avoid hitting exactly the last edge
    const G4double eps = 1e-9;
    const G4double uClamped = std::min(std::max(u, 0.0), 1.0 - eps);

    const G4double scaled = uClamped * (fN[axis] - 1);
    G4double intpart = 0.0;
    frac = std::modf(scaled, &intpart);
    idx  = static_cast<size_t>(intpart);

    // Safety: ensure idx is within [0, N-2] for linear interpolation
    if (idx >= fN[axis] - 1) {
        idx  = fN[axis] - 2;
        frac = 1.0;
    }
}

void comptonMagneticField::AddFieldValue(const G4double point[4], G4double* field) const
{
    if (!IsInBoundingBox(point)) return;

    const G4double xv = point[kX] - fOffset[kX];
    const G4double yv = point[kY] - fOffset[kY];
    const G4double zv = point[kZ] - fOffset[kZ];

    G4double xFrac[__NDIM] = {0.0, 0.0, 0.0};
    size_t   idx[__NDIM]   = {0, 0, 0};

    ComputeIndexAndFraction(xv, kX, idx[kX], xFrac[kX]);
    ComputeIndexAndFraction(yv, kY, idx[kY], xFrac[kY]);
    ComputeIndexAndFraction(zv, kZ, idx[kZ], xFrac[kZ]);

    // Decide interpolation type; force linear near edges
    EInterpolationType type = fInterpolationType;

    // For cubic, we need idx in [1, N-3] so that idx + {-1,0,1,2} stays in [0, N-1]
    bool canDoCubic = true;
    for (size_t cidx = 0; cidx < __NDIM; ++cidx) {
        if (fN[cidx] < 4) { canDoCubic = false; break; }
        if (idx[cidx] < 1 || idx[cidx] > fN[cidx] - 3) { canDoCubic = false; break; }
    }
    if (type == kCubic && !canDoCubic) {
        type = kLinear;
    }

    size_t n = 0;
    const char (*map)[3] = nullptr;
    switch (type) {
        case kLinear:
            map = kLinearMap;
            n   = 8;
            break;
        case kCubic:
            map = kCubicMap;
            n   = 64;
            break;
    }

    // Collect vertex values safely
    thread_local G4double values[__NDIM][64];

    for (size_t i = 0; i < n; ++i) {
        for (size_t cidx = 0; cidx < __NDIM; ++cidx) {
            // Compute candidate indices
            int xc = static_cast<int>(idx[kX]) + static_cast<int>(map[i][kX]);
            int yc = static_cast<int>(idx[kY]) + static_cast<int>(map[i][kY]);
            int zc = static_cast<int>(idx[kZ]) + static_cast<int>(map[i][kZ]);

            // Clamp to valid range to avoid OOB
            xc = std::max(0, std::min(xc, static_cast<int>(fN[kX]) - 1));
            yc = std::max(0, std::min(yc, static_cast<int>(fN[kY]) - 1));
            zc = std::max(0, std::min(zc, static_cast<int>(fN[kZ]) - 1));

            values[cidx][i] = fBFieldData[cidx][static_cast<size_t>(xc)]
                [static_cast<size_t>(yc)]
                [static_cast<size_t>(zc)];
        }
    }

    // Interpolate
    G4ThreeVector Bcart(0.0, 0.0, 0.0);
    for (int cidx = 0; cidx < __NDIM; ++cidx) {
        switch (type) {
            case kLinear:
                Bcart[cidx] = _trilinearInterpolate(values[cidx], xFrac);
                break;
            case kCubic:
                Bcart[cidx] = _tricubicInterpolate(values[cidx], xFrac);
                break;
        }
    }

    // Scale field
    Bcart *= fFieldScale * fGlobalScale;

    // Add to original field (if you want full vector, uncomment y/z)
    field[0] += Bcart.x();
    field[1] += Bcart.y();
    field[2] += Bcart.z();
}

