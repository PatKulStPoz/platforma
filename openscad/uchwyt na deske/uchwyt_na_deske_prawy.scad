nakladka_sz = 38; //31.4
nakladka_dl = 60;
nakladka_h = 32.5;
dno_grubosc = 7.75;

rama_sr = 25.8;

sruba_sr = 5;
sruba_od_dna = 15.35;
sruba_od_boku = 12.7;
sruby_odstep = 35;

mocowanie_h = 37.5; //35
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

mirror([1,0,0]){
union(){
    difference(){
        cube([nakladka_dl, nakladka_sz, nakladka_h], center=true);
        
        translate([0,0,-nakladka_h/2+dno_grubosc+rama_sr/2]){
        rotate([0,90,0]){
            cylinder(h=nakladka_dl+30, d=rama_sr, center=true);
            
        }
        }
        
        translate([0,0,dno_grubosc+rama_sr/2]){
            cube([nakladka_dl+30, rama_sr, nakladka_h],center = true);
        }
        rotate([-90,0,0]){
        translate([sruby_odstep/2,0,0]){
            sruba_dziura(sruba_sr);
            
            translate([-sruby_odstep,0,0]){
               sruba_dziura(sruba_sr); 
            }}
            

            
        }
        translate([-nakladka_dl/2+20,0,-nakladka_h/2+dno_grubosc+rama_sr/2]){
            rotate([0,0,0]){
                sruba_dziura(7); 
            }
        }

        
    }

    translate([0,nakladka_sz/2-mocowanie_gr/2,nakladka_h/2+mocowanie_h/2]){
        mocowanie(mocowanie_h, mocowanie_sr1, mocowanie_gr, mocowanie_dl, 4.75);
    }

}
}