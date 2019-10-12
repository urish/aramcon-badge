/*
 * 3D Model for EByte E73-2G4M08S1C
 * Copyright (C) 2019, Uri Shaked.
 *
 * To convert to a KiCad model:
 * 1. Open with OpenSCAD, export as a CSG file
 * 2. Open the CSG file with FreeCad, select all (CTRL+A), export (CTRL+E) and save as VRML V2.0
 * 3. When importing in KiCad, set scale to 0.3937 along all the axes (X, Y, and Z). 
 *    Set Z-rotation to -90 deg, X offset to 6.5mm, and Y offset to -9mm.
 */

include <MCAD/units.scad>;

$fn = 32;

module pad() {
    union() {
        color("gold")
        linear_extrude(0.01)
        difference() {
            translate([0, -0.8/2])
            square([1, 0.8]);
            
            circle(0.65/2);
        }

        color("gold")
        linear_extrude(1)
        difference() {
            translate([0, -0.8/2])
            square([0.5, 0.8]);
            
            circle(0.65/2);
        }
    }
}

module pad_hole() {
    translate([0, -0.8/2])
    square([0.5, 0.8]);
}

module lay_pads() {
    for (i = [0:7])
        translate([0, 2.06 + i * 1.27])
        children();

    for (i = [0:9])
        translate([2.6 + i * 1.27, 0])
        rotate([0, 0, 90])
        children();

    for (i = [0:9])
        translate([2.6 + i * 1.27, 13])
        rotate([0, 0, 270])
        children();
}

color("green")
translate([0, 0, epsilon])
linear_extrude(1-epsilon)
difference() {
    square([18, 13]);
    
    lay_pads() {
        pad_hole();
    }
}

lay_pads() {
    pad();
}

// Bottom pads
for (i = [0:6])
    translate([2.60 - 1.27 + 0.8/4, 2.06 + 1.27 * i + 0.8 / 4, 0])
    color("gold")
    cube([0.8, 0.8, epsilon]);

for (i = [0:7])
    translate([2.60 + 1.27 * (i + 1) + 0.8 / 4, 1.27 + 0.8/4, 0])
    color("gold")
    cube([0.8, 0.8, epsilon]);

// RF Shield
translate([1.4, (13-10.15)/2, 1])
color("silver")
cube([12.15, 10.15, 2]);

// Antenna
color("white")
translate([15.75, 2.75, 1])
cube([1.8, 8.5, 1.4]);