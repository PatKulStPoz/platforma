przycisk_h = 20;
przycisk_dl =41;
przycisk_sz = 30;

trzon_sz = 15;
trzon_dl = 30;
trzon_h = 25;

nakladka_sz = 40; //31.4
nakladka_dl = 50;
nakladka_h = 32.5;
dno_grubosc = 7.75;

rama_sr = 22.5;

wkret_sr = 2.8;

sruba_sr = 5;
sruba_od_dna = 15.35;
sruba_od_boku = 12.7;
sruby_odstep = 37;

mocowanie_h = 45; //35
mocowanie_sr1 = 10; // lub 12
mocowanie_gr = (nakladka_sz-rama_sr)/2;
mocowanie_dl = 35;



module sruba_dziura(sr){
    rotate([90,0,0]){
        cylinder(h=nakladka_sz+30, d=sr, center=true, $fn=20);
        
    }
}

module mocowanie(h, sr1, gr, dl, sr2){
    difference(){
    cube([dl, gr, h],center = true);
    
  
    translate([0,0,h/2-10-sr2/2]){
    rotate([90,0,0]){
    cylinder(gr+10, d=sr2, center=true, $fn=20);
    }
    }
    
    }
}

union(){
    difference(){
        cube([nakladka_dl, nakladka_sz, nakladka_h+przycisk_h], center=true);
        
        translate([0,0,-nakladka_h/2+dno_grubosc+rama_sr/2]){
        rotate([0,90,0]){
            cylinder(h=nakladka_dl+30, d=rama_sr, center=true);
            
        }
        }
        
         translate([0,0,nakladka_h]){
            cube([nakladka_dl+30, nakladka_sz+30, nakladka_h],center = true);
        }
        
        translate([0,0,dno_grubosc+rama_sr/2]){
            cube([nakladka_dl+30, rama_sr-2, nakladka_h],center = true);
        }
        
        rotate([20,0,0]){
                translate([0,0,dno_grubosc+rama_sr/2]){
            cube([nakladka_dl+30, rama_sr-10, nakladka_h],center = true);
        }}
        rotate([-20,0,0]){
                translate([0,0,dno_grubosc+rama_sr/2]){
            cube([nakladka_dl+30, rama_sr-10, nakladka_h],center = true);
        }}
        
         
        
        rotate([0,0,0]){
        translate([sruby_odstep/2,0,-nakladka_h/2+dno_grubosc+rama_sr/2]){
            sruba_dziura(sruba_sr);
            
            translate([-sruby_odstep,0,0]){
               sruba_dziura(sruba_sr); 
            }}
            

            
        }
        
        translate([0,nakladka_sz/2-przycisk_sz/2+5,0]){
        translate([0,0,-nakladka_h/2-10]){
            #cube([przycisk_dl, przycisk_sz, przycisk_h],center = true);
        }
        translate([0,nakladka_sz/2-trzon_sz/2,-4]){
            #cube([trzon_dl, trzon_sz, trzon_h],center = true);
        }
           rotate([-90,0,0]){
        translate([przycisk_dl/2-2.5,45,-przycisk_sz/2+5.5]){
            sruba_dziura(wkret_sr);
        }
            translate([-(przycisk_dl/2-2),45,-(-przycisk_sz/2+7.5)]){
               #sruba_dziura(wkret_sr); 
            }

    }

    
    }
}


}
