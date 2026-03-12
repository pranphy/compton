#ifndef __COMPTONMAGNETICFIELD_HH
#define __COMPTONMAGNETICFIELD_HH

#include <vector>

#define __NDIM 3

#include "G4String.hh"
#include "G4MagneticField.hh"

class comptonMagneticField : public G4MagneticField {
    /*!
     * File format:
     *   Nx  xmin xmax xoffset
     *   Ny  ymin ymax yoffset
     *   Nz  zmin zmax zoffset
     *   scale
     *   x y z bx by bz   (repeated for all grid points)
     */

    private:
        enum EInterpolationType { kLinear, kCubic };

    public:
        comptonMagneticField(const G4String& name, const G4String& filename);
        virtual ~comptonMagneticField() { }

        void AddFieldValue(const G4double point[4], G4double* field) const;
        void AddFieldValue2(const G4double point[4], G4double* field) const;
        void GetFieldValue(const G4double point[4], G4double* field) const override;

        void SetFieldScale(G4double scale) { fFieldScale = scale; }
        void SetRefCurrent(G4double current) { fRefCurrent = current; }
        void SetCurrent(G4double current) { if (fRefCurrent != 0.0) SetFieldScale(current / fRefCurrent); }
        void SetGlobalFieldScale(G4double scale) { fGlobalScale = scale; }

        void SetXOffset(G4double x) { fOffset[kX] = x; }
        void SetYOffset(G4double y) { fOffset[kY] = y; }
        void SetZOffset(G4double z) { fOffset[kZ] = z; }

        const G4String& GetName() const { return fName; }

        enum Coord_t { kX = 0, kY, kZ };

        G4bool IsInBoundingBox(const G4double* p) const {
            // Slightly shrink the upper bound to avoid mapping exactly to the last cell edge
            const G4double eps = 1e-9;
            if (p[0] < fMin[kX] + fOffset[kX] || p[0] > fMax[kX] + fOffset[kX] - eps) return false;
            if (p[1] < fMin[kY] + fOffset[kY] || p[1] > fMax[kY] + fOffset[kY] - eps) return false;
            if (p[2] < fMin[kZ] + fOffset[kZ] || p[2] > fMax[kZ] + fOffset[kZ] - eps) return false;
            return true;
        }

        G4String fName;
        G4String fFilename;

    private:
        size_t   fN[__NDIM];
        G4double fOffset[__NDIM];
        G4double fUnit[__NDIM];
        G4double fMin[__NDIM];
        G4double fMax[__NDIM];
        G4double fStep[__NDIM];
        G4double fFileMin[__NDIM];
        G4double fFileMax[__NDIM];

        std::vector<std::vector<std::vector<G4double>>> fBFieldData[__NDIM];

        G4double fXMapOffset;
        G4double fZMapOffset;
        G4double fYMapOffset;

        G4double fFieldValue;
        G4double fFieldScale; // Scale overall field by this amount
        G4double fGlobalScale; // Global scale set by global field
        G4double fRefCurrent; // Reference current for magnetic field

        EInterpolationType fInterpolationType;

    public:
        void SetInterpolationType(EInterpolationType type) { fInterpolationType = type; }
        void SetInterpolationType(const G4String& type) {
            if (type == "linear") SetInterpolationType(kLinear);
            else if (type == "cubic") SetInterpolationType(kCubic);
        }
        EInterpolationType GetInterpolationType() const { return fInterpolationType; }

    private:
        static const char kLinearMap[8][3];
        static const char kCubicMap[64][3];

        G4double field_x(G4double z) const;

        // Interpolation helpers
        double _linearInterpolate(const double p[2 << 0], double x) const {
            return p[0] + x * (p[1] - p[0]);
        }
        double _bilinearInterpolate(const double p[2 << 1], const double x[2]) const {
            double c[2];
            c[0] = _linearInterpolate(&(p[0]), x[0]);
            c[1] = _linearInterpolate(&(p[2]), x[0]);
            return _linearInterpolate(c, x[1]);
        }
        double _trilinearInterpolate(const double p[2 << 2], const double x[3]) const {
            double c[2];
            c[0] = _bilinearInterpolate(&(p[0]), &(x[0]));
            c[1] = _bilinearInterpolate(&(p[4]), &(x[0]));
            return _linearInterpolate(c, x[2]);
        }

        double _cubicInterpolate(const double p[4 << 0], double x) const {
            return p[1] + 0.5 * x *
                (p[2] - p[0] +
                 x * (2. * p[0] - 5. * p[1] + 4. * p[2] - p[3] +
                     x * (3. * (p[1] - p[2]) + p[3] - p[0])));
        }
        double _bicubicInterpolate(const double p[4 << 1], const double x[2]) const {
            double c[4];
            c[0] = _cubicInterpolate(&(p[0]),  x[1]);
            c[1] = _cubicInterpolate(&(p[4]),  x[1]);
            c[2] = _cubicInterpolate(&(p[8]),  x[1]);
            c[3] = _cubicInterpolate(&(p[12]), x[1]);
            return _cubicInterpolate(c, x[0]);
        }
        double _tricubicInterpolate(const double p[4 << 2], const double x[3]) const {
            double c[4];
            c[0] = _bicubicInterpolate(&(p[0]),  &(x[1]));
            c[1] = _bicubicInterpolate(&(p[16]), &(x[1]));
            c[2] = _bicubicInterpolate(&(p[32]), &(x[1]));
            c[3] = _bicubicInterpolate(&(p[48]), &(x[1]));
            return _cubicInterpolate(c, x[0]);
        }

        // Safe index computation: returns base index and fractional coordinate
        void ComputeIndexAndFraction(G4double coord,
                size_t axis,
                size_t& idx,
                G4double& frac) const;
};

#endif // __COMPTONMAGNETICFIELD_HH

