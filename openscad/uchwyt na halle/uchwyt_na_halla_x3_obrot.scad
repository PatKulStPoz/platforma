true_grubosc = 30;
kat = 10;

grubosc = true_grubosc*2;
grubosc_skrzydelek = 7;
szerokosc_hall = 14.2;
dlugosc_hall = 31.2;
odstep = 1.5;
wkret_sr = 2;

sz_plytka = 15.24;
dl_plyrka = 41.91;
odstep_plytka = 6.2;
plytka_marg = 1;

granica_x = 56;
granica_y = 65;

module wanna(h){
    difference(){
        cube([20,50,h],center = true);
        
        translate([0,0,h/2]){
        cube([15, 33, 4],center=true);
        }
    }
}

module trzon(){
    difference(){
    cube([20,65,grubosc-grubosc_skrzydelek*2],center = true);
    
    translate([0,0,-20]){
        rotate([90,0,0]){
            #cylinder(h=80, r=20, center=true);
        }
    }
    
    translate([0,25,5]){
        rotate([0,90,0]){
            #cylinder(h=60, r=2, center=true, $fn=10);
        }
    }
        translate([0,-25,5]){
        rotate([0,90,0]){
            #cylinder(h=60, r=2, center=true, $fn=10);
        }
    }
}
}

module wkret(){
    translate([0,15.6-7,grubosc/2]){
      cylinder(h=10, d=wkret_sr, center=true, $fn=10);
    }
}


difference(){
union(){
    rotate([0,0,kat]){
union(){
difference(){
union(){

    translate([0,10,grubosc/2-grubosc_skrzydelek/2]){
    cube([70,80,grubosc_skrzydelek], center = true);
    }
    //translate([0,0,grubosc/2]){
      //  #cube([15, 33, 4],center=true);}
}
        translate([0,0,grubosc/2]){
        cube([szerokosc_hall*3+odstep*2, dlugosc_hall+0.4, 4],center=true);
    }
    
    wkret();
    
    translate([15,0,0]){
    wkret();
    }
    
     translate([-15,0,0]){
    wkret();
    }
    
    translate([0,(dlugosc_hall+0.4)/2-2,grubosc/2])
    cube([dl_plyrka+plytka_marg, 4, 4+4],center=true);
    
    translate([0,-(dlugosc_hall+0.4)/2+2.5,grubosc/2])
    cube([szerokosc_hall*3+odstep*2, 5, 4+4],center=true);
    
    translate([0,-4.5+odstep_plytka,0]){
    translate([0,28,grubosc/2])
    cube([dl_plyrka+plytka_marg, sz_plytka+plytka_marg, 4],center=true);
    
    translate([dl_plyrka/2-3.81,28-15.6+7+sz_plytka/2-5.08,0]){
    wkret();
    }
    
    translate([-dl_plyrka/2+3.81,28-15.6+7+sz_plytka/2-5.08,0]){
    wkret();
    }
    
    translate([0,28-(sz_plytka+plytka_marg)/2+3.5,grubosc/2])
    cube([dl_plyrka+plytka_marg, 7, 4+4],center=true);
    
    translate([0,28,grubosc/2])
    cube([18, sz_plytka+plytka_marg, 4+4],center=true);
    
    translate([0,18,grubosc/2])
    cube([dl_plyrka+plytka_marg, 8, 4+4],center=true);
}


}
    translate([8,10,grubosc/2-3])
    cube([2, 65, 2],center=true);

    translate([-8,10,grubosc/2-3])
    cube([2, 65, 2],center=true);
}
}
   translate([0,10,0])
   trzon();
}
    translate([0,-granica_y/2,0])
    #cube([100,20,grubosc+10],center=true);

    translate([0,granica_y/2+20,0])
    #cube([100,20,grubosc+10],center=true);

    translate([granica_x/2+10,0,0])
    #cube([20,100,grubosc+10],center=true);

    translate([-granica_x/2-10,0,0])
    #cube([20,100,grubosc+10],center=true);

}