szprycha_srednica = 2.2;
szprycha_dlugosc = 143;
szprychy_odstep = 102;
kat = acos((2*szprycha_dlugosc*szprycha_dlugosc-szprychy_odstep*szprychy_odstep)/(2*szprycha_dlugosc*szprycha_dlugosc));

uchwyt_dlugosc = 45;
uchwyt_szerokosc = 10;
uchwyt_grubosc = 5;
uchwyt_wysokosc = 50;
magnes_h = 3;
magnes_d = 8;

difference(){
    union(){
        translate([0,0,uchwyt_wysokosc]){
            cube([uchwyt_dlugosc,uchwyt_grubosc,uchwyt_szerokosc],center=true);
            translate([-uchwyt_dlugosc/2,0,0]){
            rotate([90,0,0]){
                cylinder(h=uchwyt_grubosc, d=uchwyt_szerokosc, center=true);
            }
        }
                    translate([uchwyt_dlugosc/2,0,0]){
            rotate([90,0,0]){
                cylinder(h=uchwyt_grubosc, d=uchwyt_szerokosc, center=true);
            }
        }
        }
    }
    
    translate([0,uchwyt_grubosc/2-magnes_h-0.2,uchwyt_wysokosc])
    {
        rotate([-90,0,0]){
            #cylinder(h = 10, d=magnes_d+0.2);
        }
    }
    
    
    rotate([0,kat/2,0]){
    #cylinder(h= szprycha_dlugosc, d=szprycha_srednica, center = false, $fn=20);
    }
    rotate([0,-kat/2,0]){
    translate([0,0,0]){
        #cylinder(h= szprycha_dlugosc, d=szprycha_srednica, center = false, $fn=20);
    }
    }
    
    translate([0, -szprycha_srednica*0.5,0]){
    rotate([0,kat/2,0]){
    #cylinder(h= szprycha_dlugosc, d=szprycha_srednica*0.9, center = false, $fn=20);
    }
    rotate([0,-kat/2,0]){
    translate([0,0,0]){
        #cylinder(h= szprycha_dlugosc, d=szprycha_srednica*0.9, center = false, $fn=20);
    }
    }
    }
        translate([0, -szprycha_srednica,0]){
    rotate([0,kat/2,0]){
    #cylinder(h= szprycha_dlugosc, d=szprycha_srednica*0.9, center = false, $fn=20);
    }
    rotate([0,-kat/2,0]){
    translate([0,0,0]){
        #cylinder(h= szprycha_dlugosc, d=szprycha_srednica*0.9, center = false, $fn=20);
    }
    }
    }
}