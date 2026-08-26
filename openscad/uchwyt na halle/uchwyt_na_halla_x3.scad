grubosc = 45;
grubosc_skrzydelek = 7;
odstep = 4;

module wanna(h){
    difference(){
        cube([20,50,h],center = true);
        
        translate([0,0,h/2]){
        #cube([15, 33, 4],center=true);
        }
    }
}

module trzon(){
    difference(){
    cube([20-odstep,50,grubosc],center = true);
    
    translate([0,0,-20]){
        rotate([90,0,0]){
            #cylinder(h=60, r=20, center=true);
        }
    }
    translate([0,0,grubosc/2]){
        #cube([15, 33, 4],center=true);
    }
    
    translate([0,0,0]){
        #cylinder(h=60, d=2, center=true);
    }
    
    translate([0,21,5]){
        rotate([0,90,0]){
            #cylinder(h=60, r=2, center=true);
        }
    }
        translate([0,-21,5]){
        rotate([0,90,0]){
            #cylinder(h=60, r=2, center=true);
        }
    }
}
}

union(){
trzon();

translate([17.5,0,grubosc/2-grubosc_skrzydelek/2]){
    wanna(grubosc_skrzydelek);
}

translate([-17.5,0,grubosc/2-grubosc_skrzydelek/2]){
    wanna(grubosc_skrzydelek);
}

}