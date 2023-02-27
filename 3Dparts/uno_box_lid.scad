wallthickness = 3;
bottomthickness = 6;

length = 90;
width = 62;
height = 53;

squeezeFactor = 0;
LEDholeradius = 4;

depth =3;

inner_width  = width-(wallthickness*2)-squeezeFactor;
inner_height = height-(wallthickness+bottomthickness) - squeezeFactor;

difference(){
cube([depth,inner_width,inner_height]);
    union() {
         translate([-1,inner_width/2,inner_height*0.85]) 
         rotate([0,90,0])
         cylinder(h=10,r=LEDholeradius,$fn = 60);
         
         // For the cables: From y = 9 to 40 mm, from z = 0 to 25 mm
         translate([-1,9,-1])
         cube([10,31,25+1]);
        
        // Hole to put a (small) screwdriver in to pull it out
        // 4 mm by 2 mm
        translate([-1,inner_width-2,inner_height/2-2])
        cube([10,2+1,4]);
    }
}