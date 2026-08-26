
    difference() {
        cylinder(h=14, r=6, center = false);
        translate([-6,-1.1,6]){
            cube([12,2.2,10], center = false);
        }
        translate([0,0,-1]){
            cylinder(h=3, r=4.75, center=false);
        }
        translate([0,0,10]){
            cylinder(h=5, r=4, center=false, $fn=6);
        }
    }