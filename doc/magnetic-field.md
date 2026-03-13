# Magnetic Field

The experimental setup consists of four dipole magnets, each with a strength of approximately **1.5 Tesla**.

## Magnetic Field Maps

The magnetic field maps can be downloaded from the following link:
[http://userweb.jlab.org/~pgautam/downloads/compton](http://userweb.jlab.org/~pgautam/downloads/compton)

The available map files follow this naming convention:
- `dipole_magnetic_field_${FIELD_ID}_1.txt`
- `dipole_magnetic_field_${FIELD_ID}_2.txt`
- `dipole_magnetic_field_${FIELD_ID}_3.txt`
- `dipole_magnetic_field_${FIELD_ID}_4.txt`

### Field IDs (`FIELD_ID`)

The `${FIELD_ID}` in the filenames indicates the type of magnetic field configuration:
- **`20`**: Basic dipole field
- **`21`**: Shim field

## Loading the Fields

The `comptonGlobalField` class handles the loading of all magnetic fields. Corresponding to the magnetic field ID, the fields are loaded using macro commands.

The macro command `/compton/field/add <ID> <fieldpath>` has the following structure:
1. `<ID>` corresponds to the assigned ID in the GDML for the magnet.
2. `<fieldpath>` is the path to the magnetic field simulation file.

### GDML Definition and Field ID

In the GDML file, the magnetic volume is defined with an `auxiliary` tag for the magnetic field. The value assigned to the `MagField` auxiliary tag corresponds directly to the `<ID>` used in the macro command.

For example:
```xml
    <volume name="dipole_magnet_1_lv_n">
      <materialref ref="G4_Galactic"/> <!-- This is ok vacuum because it is larger than the magnet, this is region -->
      <solidref ref="magnetic_region"/>
      <auxiliary auxtype="Alpha" auxvalue="0.0"/>
      <auxiliary auxtype="stepl" auxvalue="1" />
      <auxiliary auxtype="MagField" auxvalue="1" />
      <physvol name="magnet_1">
        <volumeref  ref="dipole_magnet_1_lv"/>
      </physvol>
    </volume>
```
In this snippet, `<auxiliary auxtype="MagField" auxvalue="1" />` sets the field ID to `1`. This is the ID that will be used in the `/compton/field/add` command.

Example macro commands to load the magnetic fields:
```
/compton/field/add 1 map_directory/20/dipole_magnetic_field_20_1.txt
/compton/field/add 2 map_directory/20/dipole_magnetic_field_20_2.txt
/compton/field/add 3 map_directory/20/dipole_magnetic_field_20_3.txt
/compton/field/add 4 map_directory/20/dipole_magnetic_field_20_4.txt
```

### Global Scale Tuning

The command `/compton/field/globalscale <value>` sets the global scale for all the magnetic field values obtained from the files.

For example:
```
/compton/field/globalscale 0.97886
```
This is done to tune each of the four magnets at once with a single parameter. In practice, this corresponds to the adjustment of magnetic field strength to get the beam precisely at the laser table.
