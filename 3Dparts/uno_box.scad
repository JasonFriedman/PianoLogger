wallthickness = 3;
bottomthickness = 6;

length = 90;
width = 62;
height = 53;

holewidth = 48;
hole1radius = 2;
holesidedistance = (width - holewidth)/2;
hole2radius = 3;

difference(){ 
    cube([length,width,height]);
    union() {
    translate([3+wallthickness,wallthickness,bottomthickness])
    cube([length-(wallthickness*2)+1,width-(wallthickness*2),height-(wallthickness+bottomthickness)]);
    
    translate([60,holesidedistance,3]) cylinder(h=10,r=hole1radius);
    translate([60,holesidedistance,-1]) cylinder(h=4,r=hole2radius);
        
    translate([60,width-holesidedistance,3]) cylinder(h=10,r=hole1radius);
    translate([60,width-holesidedistance,-1]) cylinder(h=4,r=hole2radius);
    }
}

// Add 4 small pieces to stop the lid from falling in
translate([length-3-1,wallthickness,bottomthickness])
cube([1,3,3]);

translate([length-3-1,width-wallthickness-3,bottomthickness])
cube([1,3,3]);

translate([length-3-1,wallthickness,height-3-wallthickness])
cube([1,3,3]);

translate([length-3-1,width-wallthickness-3,height-3-wallthickness])
cube([1,3,3]);
