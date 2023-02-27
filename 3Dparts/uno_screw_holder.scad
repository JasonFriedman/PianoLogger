// Note: this requires the library from here: https://github.com/b33j0r/Metric
use <Metric/M2.scad>;

uno_width = 53;
width = uno_width + 1;

depth = 5;
height = 10;

holedepth = depth;
holewidth = 42;
holeheight = 6;

holedistance = 48;

difference() {
    cube([depth,width,height]);
    union() {
        translate([-1,(width-holewidth)/2,-1])
        cube([depth+2,holewidth,holeheight+1]);
        
        translate([depth/2,(width-holedistance)/2,1.55])
        NutM2();
        
        translate([depth/2,width-(width-holedistance)/2,1.55])
        NutM2();


    }
}
