include <BOSL2/std.scad>
include <BOSL2/threading.scad>

$fn = 64;

d_sruby = 6.73+0.5;     // zmierzona średnica zewnętrzna śruby
pitch   = 1;     // skok gwintu

// luz
luz = 0.15;


difference(){
 cylinder(h = 6, d = d_sruby+3, $fn=6, center = true);
 threaded_rod(d=d_sruby+luz, l=15, pitch=pitch, orient = UP);
}