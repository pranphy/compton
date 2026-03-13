# Parallel Plane Detectors

The `geometry/compton/ComptonParallel.gdml` defines several virtual planes used to track particles (detector IDs are specified using the `<auxiliary auxtype="DetNo">` tag).

Below is the layout of these virtual planes and their corresponding detector IDs, ordered along the Z-axis:

| Physical Volume | Logical Volume | Detector ID | Z Position (mm) |
| :--- | :--- | :--- | :--- |
| `ZVP_phy` | `Zeroth_vp` | 10 | -7950 |
| `FVP_phy` | `First_vp` | 11 | -7549.77 |
| `second_phy` | `second_vp` | 12 | -5718.048 |
| `valve_ent_phy` | `valve_ent` | 42 | -3953.0 |
| `valve_ext_phy` | `valve_ext` | 43 | -3947.0 |
| `Sync_Che_phy` | `Sync_Check_VP` | 41 | -2950 |
| `orig_phy` | `orig_vp` | 13 | 0 |
| `dipole_tree_US_phy` | `dipole_three_US` | 301 | 1144.92 |
| `dipole_three_ds_phy` | `dipole_three_DS` | 302 | 2146.92 |
| *(unnamed)* | `weldmend_3_exit` | 304 | 2346.92 |
| `VRVP_phy` | `VRight_vp` | 14 | 165.12*25.4 |
| *(unnamed)* | `weldmend_US_lower_entry` | 305 | 4265.00 |
| *(unnamed)* | `weldmend_US_lower_exit` | 306 | 4276.32 |
| `midway_dipole_three_ed_phy` | `midway_dipole_three_ed` | 303 | 5552.38 |
| `strip_PV` | `strip_vp` | 16 | 6057.41 |
| `strip_back_PV` | `strip_vp_back` | 17 | 6060.41 |
| `TVP_phy` | `Third_vp` | 15 | 7800 |

*(Note: `165.12 * 25.4` mm is exactly `4194.048` mm)*
