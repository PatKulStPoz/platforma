pod_h = 6;
pod_x = 106.2;
pod_y = 85.7;

sruba_sr = 2.2;
wkret_off_x = 6.2;
wkret_off_y = 6.2;

wkret_sr = 4.25;
marg = 20;


podpora_h = 10;
podpora_x = 12;

module podpora(){
    cube([podpora_x,podpora_x,podpora_h],center=true);
}

module wkret(sr){
    cylinder(h = 50, d = sr, center=true,$fn=30);
}

difference(){
union(){
    cube([pod_x,pod_y,pod_h], center=true);
    
    difference(){
    translate([pod_x/2-podpora_x/2,pod_y/2-podpora_x/2,pod_h]){
        podpora();
    }
    translate([pod_x/2-wkret_off_x-1.2,pod_y/2-wkret_off_y-1.2,pod_h]){
        wkret(sruba_sr);
    }
}
    difference(){
    translate([-(pod_x/2-podpora_x/2),-(pod_y/2-podpora_x/2),pod_h]){
        podpora();
    }
        translate([-(pod_x/2-wkret_off_x-1.2),-(pod_y/2-wkret_off_y-1.2),pod_h]){
        wkret(sruba_sr);
    }
}
    difference(){
    translate([-(pod_x/2-podpora_x/2),pod_y/2-podpora_x/2,pod_h]){
        podpora();
    }
        translate([-(pod_x/2-wkret_off_x-1.2),pod_y/2-wkret_off_y-1.2,pod_h]){
        wkret(sruba_sr);
    }
}
    difference(){
    translate([pod_x/2-podpora_x/2,-(pod_y/2-podpora_x/2),pod_h]){
        podpora();
    }
        translate([pod_x/2-wkret_off_x-1.2,-(pod_y/2-wkret_off_y-1.2),pod_h]){
        wkret(sruba_sr);
    }
}
}
    translate([pod_x/2-marg,0,0]){
        wkret(wkret_sr);
    }
    
    translate([-pod_x/2+marg,0,0]){
        wkret(wkret_sr);
    }
}