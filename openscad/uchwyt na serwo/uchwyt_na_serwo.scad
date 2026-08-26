include <BOSL2/std.scad>
include <BOSL2/threading.scad>

$fn = 64;

d_sruby = 6.73+0.5;     // zmierzona średnica zewnętrzna śruby
pitch   = 1;     // skok gwintu

// luz
luz = 0.15;

//serwo
ser_x = 40.5;
ser_y = 40;
ser_h = 39.2;

//uchwyt

od_gory = 15;
u_x = ser_x+20;
u_y = 17.5;
u_z = 40;
grubosc = 6;

sr_deska = 4.25;
sr_serwo = 2;

linka_odl = 12.5;
linka_h = 25;


module wkret(sr){
    cylinder(d=sr, h=20, center = true, $fn = 50);
}

union(){
difference(){
    translate([0,-10,0]){
    cube([u_x,u_y+20,u_z], center = true);
    }
    translate([0,-u_y-5/2,-grubosc]){
        cube([u_x,u_y+5,u_z], center = true);
    }
    
        translate([0,u_y/2,u_z/2-ser_y/2-od_gory]){
        cube([ser_x+1,ser_h,ser_y], center = true);
    }
    
    translate([-20,-17.5,u_z/2]){
    wkret(sr_deska);
    }
    
        translate([20,-17.5,u_z/2]){
    wkret(sr_deska);
    }
    
    translate([(ser_x+1)/2+2,15,-1]){
        rotate([90,0,0]){
    wkret(sr_serwo);
            
       translate([0,-5-4.5,0]){
        wkret(sr_serwo);
           }
    }}
    
        translate([-(ser_x+1)/2-2,15,-1]){
        rotate([90,0,0]){
    wkret(sr_serwo);
            
       translate([0,-5-4.5,0]){
        wkret(sr_serwo);
           }
    }}
}
    difference(){
    translate([u_x/2+linka_odl/2,u_y/2-5/2,0]){
    cube([linka_odl,5,u_z], center = true);
    translate([linka_odl/2+2.5,linka_h/2-2.5,0]){
        cube([5,linka_h,u_z], center = true);}
    }
    translate([u_x/2+linka_odl/2+10,u_y/2-5/2+15,-1]){
    #threaded_rod(d=d_sruby+luz, l=15, pitch=pitch, orient = RIGHT);}
}
}