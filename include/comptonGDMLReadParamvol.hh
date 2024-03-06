#pragma once

#include "G4UnitsTable.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4AssemblyVolume.hh"
#include "G4ReflectionFactory.hh"
#include "G4PVDivisionFactory.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4VisAttributes.hh"

#include "G4GDMLReadParamvol.hh"

class comptonGDMLReadParamvol : public G4GDMLReadParamvol {
    private:
    void ParametersRead(const xercesc::DOMElement* const element) {

        G4ThreeVector rotation(0.0, 0.0, 0.0);
        G4ThreeVector rotationyzx(0.0, 0.0, 0.0);
        G4ThreeVector position(0.0, 0.0, 0.0);
        bool yzx_requested = false;

        G4GDMLParameterisation::PARAMETER parameter;
        parameter.pRot = new G4RotationMatrix();

        for(xercesc::DOMNode* iter = element->getFirstChild(); iter != nullptr;
                iter                   = iter->getNextSibling())
        {
            if(iter->getNodeType() != xercesc::DOMNode::ELEMENT_NODE)
            {
                continue;
            }

            const xercesc::DOMElement* const child =
                dynamic_cast<xercesc::DOMElement*>(iter);
            if(child == nullptr)
            {
                G4Exception("G4GDMLReadParamvol::ParametersRead()", "InvalidRead",
                        FatalException, "No child found!");
                return;
            }
            const G4String tag = Transcode(child->getTagName());
            if(tag == "rotation")
            {
                VectorRead(child, rotation);
            }
            else if(tag == "rotationyzx")
            {
                std::cout<<" Requested rotation yzx"<<std::endl;
                VectorRead(child, rotationyzx);
                yzx_requested = true;
            }
            else if(tag == "position")
            {
                VectorRead(child, position);
            }
            else if(tag == "positionref")
            {
                position = GetPosition(GenerateName(RefRead(child)));
            }
            else if(tag == "rotationref")
            {
                rotation = GetRotation(GenerateName(RefRead(child)));
            }
            else if(tag == "box_dimensions")
            {
                Box_dimensionsRead(child, parameter);
            }
            else if(tag == "trd_dimensions")
            {
                Trd_dimensionsRead(child, parameter);
            }
            else if(tag == "trap_dimensions")
            {
                Trap_dimensionsRead(child, parameter);
            }
            else if(tag == "tube_dimensions")
            {
                Tube_dimensionsRead(child, parameter);
            }
            else if(tag == "cone_dimensions")
            {
                Cone_dimensionsRead(child, parameter);
            }
            else if(tag == "sphere_dimensions")
            {
                Sphere_dimensionsRead(child, parameter);
            }
            else if(tag == "orb_dimensions")
            {
                Orb_dimensionsRead(child, parameter);
            }
            else if(tag == "torus_dimensions")
            {
                Torus_dimensionsRead(child, parameter);
            }
            else if(tag == "ellipsoid_dimensions")
            {
                Ellipsoid_dimensionsRead(child, parameter);
            }
            else if(tag == "para_dimensions")
            {
                Para_dimensionsRead(child, parameter);
            }
            else if(tag == "polycone_dimensions")
            {
                Polycone_dimensionsRead(child, parameter);
            }
            else if(tag == "polyhedra_dimensions")
            {
                Polyhedra_dimensionsRead(child, parameter);
            }
            else if(tag == "hype_dimensions")
            {
                Hype_dimensionsRead(child, parameter);
            }
            else
            {
                G4String error_msg = "Unknown tag in parameters: " + tag;
                G4Exception("G4GDMLReadParamvol::ParametersRead()", "ReadError",
                        FatalException, error_msg);
            }
        }


        if(yzx_requested){
            parameter.pRot->rotateY(rotation.y());
            parameter.pRot->rotateZ(rotation.z());
            parameter.pRot->rotateX(rotation.x());
        } else {
            parameter.pRot->rotateX(rotation.x());
            parameter.pRot->rotateY(rotation.y());
            parameter.pRot->rotateZ(rotation.z());
        }


        parameter.position = position;

        parameterisation->AddParameter(parameter);

    }


};
