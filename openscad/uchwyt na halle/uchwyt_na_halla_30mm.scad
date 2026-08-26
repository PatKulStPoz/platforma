difference(){
    cube([20,50,30],center = true);
    
    translate([0,0,-20]){
        rotate([90,0,0]){
            cylinder(h=60, r=20, center=true);
        }
    }
    translate([0,0,15]){
        cube([15, 33, 4],center=true);
    }
    
    translate([0,21,5]){
        rotate([0,90,0]){
            cylinder(h=60, r=2, center=true);
        }
    }
        translate([0,-21,5]){
        rotate([0,90,0]){
            cylinder(h=60, r=2, center=true);
        }
    }
}