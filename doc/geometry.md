# Geant4 Geometry
Geometry is essential for specifying the experimental setup hardwares and detectors.

## Geometry
`comptonDetectorConstruction` class handles the construction of geometry. Geant4 has different classes to create different shapes and properties. We can use c++ API to create geometry. There are various classes in geant4 library like `Box`, `Tube` etc to create geometry. But this process of using compiled code and hardcoded properties make it difficult for us to rapidly update the geometry.

The other approach is the use of gdml. The geometry is constructed using the `gdml` specification. The `gdml` are `xml` files. In the following sections we only describe the `gdml` way of constructing geometry.

**GDML (Geometry Description Markup Language)** is an XML-based format for describing complex geometries in a way that is independent of specific applications, facilitating geometry data exchange among various simulation

### Solid
Solid is a geometric structure without any physical properties. A cube for example is a solid.
In gdml we create a solid with different geometric shape tags. For example we can create a cube with a box tag.

```xml
<box name="prop_cuboid" unit="mm" x="1" y="2" z="3" />
```
This will create a box of size $2 	imes 4 	imes 6$mm. Refer to the gdml documentation for more details.

Similarly we can create a cylinder with a tube tag like this.

```xml
<tube name="solid_short_tube" unit="mm" rmax="1" rmin="0" z="2" />
```

### Logical Volume
We can add physical properties to a solid and it becomes a logical volume. A cube with specified material of "Aluminium" for example is a possible logical volume.
Say we want to assign the property that the `prop_cuboid` is a alimunium then we create a logical volume like so.

```xml
<volume name="prop_cuboid_aluminium">
    <solidref ref="prop_cuboid" />
    <materialref ref="G4_Al"/>
</volume>
```
Here `G4_Al` is the name of the material. This specifically is aluminium which is predefined in geant4. We can create materials of our own.

We can also specify other properties of the solid in the logical volume like colour, opacity etc. The colour, opacity etc are usually handeled using `auxvalue` tag which has to be parsed explicitly by us to process and pply the property to the solid.

### Physical Volume
If a logical volume is placed somewhere in the geometry of the simulation its called a physical volume. A physical volume is a logical volume with position information. We can in principle place the same logical volume in different positions and each would be a distinct physical volume. Think of logical volume as a something that a factory produces. We can order same shape and property object and place at different location. The tag `physvol` is used to create a physical volume.

```xml
<physvol name="prop_cuboid_aluminium">
    <volumeref="prop_cuboid" />
    <position x="0" y="0" z="0" unit="mm" />
</physvol>
```
In this example we placed the aluminium cuboid right at the origin. The `position` tag specifies the placement of the physical volume in the coordinate system of "mother volume".

### Organization
All the solids are defined in `<solid>` tag.
In principle a logical volume can contain one or more solids. The volume that contains other volumes is called mother volume.
There is always an all-encompassing volume that includes all other volumes in geant4 simulation. That all encompassing volume is called the "world volume". The world volume is the mother volume of all the geometries. In geant4 all physical volume must be contained in a mother logical volume except for the outermost "world volume".

### Example
A complete example of a `gdml` file with a single aliminum cube placed at center is

```xml
<?xml version="1.0" encoding="UTF-8" ?>

<gdml xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://service-spi.web.cern.ch/service-spi/app/releases/GDML/schema/gdml.xsd">

    <solid>
        <box name="WorldSolid" unit="mm" x="20" y="20" z="20" />
        <box name="prop_cuboid" unit="mm" x="1" y="2" z="3" />
    </solid>

    <structure>

        <volume name="prop_cuboid_aluminium">
            <solidref ref="prop_cuboid"/>
            <materialref ref="G4_Al"/>
            <auxiliary auxtype="Color" auxvalue="Blue"/>
        </volume>

        <!-- World Volume Reference -->
        <volume name="World">
            <materialref ref="G4_Galactic" />
            <solidref ref="WorldSolid" />

            <physvol name="prop_cuboid_aluminium">
                <volumeref="prop_cuboid" />
                <position x="0" y="0" z="0" unit="mm" />
            </physvol>

        </volume>
    </structure>

    <setup name="Default" version="1.0">
        <world ref="World" />
    </setup>
</gdml>
```

This is an example of complete gdml file with a cuboid placed at origin. The "World" volume is the mother volume of the cuboid volume which is placed at origin. The origin of the simulation is the center of the world volume and all other volumes are relative to world volume. If a volume has any mother volume, all children of that mother volume have coordinates relative to the mother volume.

We can also include a `xml` file into a gdml. Say we want to organize all our solid definitions into a file called `solids.xml`

```xml
<box name="WorldSolid" unit="mm" x="20" y="20" z="20" />
<box name="prop_cuboid" unit="mm" x="1" y="2" z="3" />
```

We can then update our `gdml` file as follows

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<!DOCTYPE gdml [
<!ENTITY solids SYSTEM "solids.xml">
]>

<gdml . . . . >

    <solid>
        &solids;
    </solid>

    <structure>
       <!---
        ...
       ->
    </structure>

    <setup name="Default" version="1.0">
        <world ref="World" />
    </setup>
</gdml>
```

Here we have used the `<!DOCTYPE gdml [ <! ENTITY ... ]>` tag to include the `solids.xml` file into a "variable" solids. And used that inside the `<solid>` tag inside the `gdml`. The act of doing `&solids;` basically dumps the contents of the referenced file in that location.

## Compton Geometry

The main geometry file is called `ComptonGeometry.gdml` it lives, by default in,

```
geometry/compton/ComptonGeometry.gdml
```


In the macro the path to the entry gdml is set as

```mac
/compton/geometry/set geometry/compton/ComptonGeometry.gdml
```


It is organized such that the solids are all defined in `geometry/compton/common/solids.xml`

The solids file defines various solids which are used in the geometry construction.


## Colour Parameter

The colour of some volume can be set via two different colour names.

```xml
<auxiliary auxtype="Colour" auxvalue="Green" />
<auxiliary auxtype="Alpha" auxvalue="1" />
```


This applies the solid (alpha = 1) green colour to the associated solid. There is also another method of applying colour

```xml
<auxiliary auxtype="Colour" auxvalue="#00FF00FF" />
```

This also makes solid green colour, but we have supplied the colour and (optional last two chars) alpha value hy the hex colour code.
