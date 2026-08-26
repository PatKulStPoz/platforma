d1 = 3;
dk = 5.5;
k = 3;

odstep = 110;

module uchwyt(){
    import("/home/piotr/Dokumenty/openscad/ogniwa/21700.STL");
}

module uchwytx2(){
    translate([-odstep,0,0]){
    union(){
    uchwyt();
    translate([0,0,30]){
        uchwyt();
    }
    translate([10,4.3338,22.5]){
        cube([85, 5, 10]);
    }
    /*
    color("red")
    translate([96.25,18,13]){
    rotate([0,90,0]){
    cylinder(h=k, d=dk);
    }}
    */
}
}

}

module minus(){
    cube([2,1, 7]);
}

module plus(){
    minus();
    rotate([0,90,0]){
        translate([-4.5,0,-2.5]){
        minus();}
    }
}

difference(){
union(){
    uchwytx2();

    mirror([180,0,0]){
        uchwytx2();
    }


    translate([-18,4.3338,2.5]){
        cube([36,5, 50]);
    }
    
}
    translate([5,8.5,9]){
    #plus();
    }
    
    rotate([0,0,0]){
    translate([-8,8.5,9]){
    #minus();
    }}
    
    translate([-8,8.5,39]){
    #plus();
    }
    
    rotate([0,0,0]){
    translate([5,8.5,39]){
    #minus();
    }}
    
    translate([13,0,27.5]){
        rotate([-90,0,0]){
            #cylinder(h=20, d=3.25, $fn=30);
        }
    }
        translate([-13,0,27.5]){
        rotate([-90,0,0]){
            #cylinder(h=20, d=3.25, $fn=30);
        }
    }
    
    

    
}
