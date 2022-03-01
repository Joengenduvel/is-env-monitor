$spacing = 0.4;
$double_spacing = $spacing * 2;

$comp_width = 58 + $spacing * 2;
$board_width = 52;
$board_length = 80 + $spacing * 2;

$comp_height = 11;
$port_height = 3;
$port_width = 8;
$top_clearance = 3;

$wall_thickness = 1.5;
$wall_twice = $wall_thickness * 2;

$dip_height = 3;
$board_thickness = 1.5;

$clip_width = 20;
$clip_height = 5;
$clip_wall_offset = 4;

difference()
{

    $height = ($wall_thickness) + $dip_height + ($board_thickness * 2) +
              $comp_height + $port_height;

    // Base enclosure
    roundedcube(
        [ $board_length + $wall_twice, $comp_width + $wall_twice, $height ],
        false,
        1,
        "zmin");

    // Hollow out
    translate(
        [ $wall_thickness, $wall_thickness, $wall_thickness + $dip_height ])
        roundedcube([ $board_length, $comp_width, 19 ], false, 1, "z");

    $edge_offset = 2;
    $edge_offset_twice = $edge_offset * 2;

    // Hollow out dip
    translate(
        [ $wall_thickness + $edge_offset, $wall_thickness, $wall_thickness ])
        cube([ $board_length - $edge_offset_twice, $comp_width, $dip_height ]);

    // Create hole for port
    $dist_to_port = 8.5 + 11.5;
    translate([
        $wall_thickness + $dist_to_port - $double_spacing,
        0,
        $height - $port_height -
        $double_spacing
    ])
        cube([
            $port_width + ($double_spacing * 2),
            $wall_thickness + 1,
            $port_height + ($double_spacing * 2)
        ]);

    // For test print
    // translate([0, $wall_twice * 2, ($wall_thickness * 2) + $dip_height])
    // cube([$board_length + $wall_twice, $comp_width + $wall_twice, $height]);
    // translate([0, 0, ($wall_thickness * 2) + $dip_height])
    // cube([$dist_to_port - 4, $comp_width + $wall_twice, $height]);
    // translate([$dist_to_port + $port_width + $wall_thickness +
    // $double_spacing + 4, 0, ($wall_thickness * 2) + $dip_height])
    // cube([$board_length, $comp_width + $wall_twice, $height]);

    // Create clipping points
    translate([
        0,
        ($comp_width + $wall_twice) / 2 - ($clip_width / 2),
        $height - $clip_wall_offset -
        $clip_height
    ]) cube([ $wall_twice, $clip_width, $clip_height ]);

    translate([
        $board_length + $wall_thickness,
        ($comp_width + $wall_twice) / 2 - ($clip_width / 2),
        $height - $clip_wall_offset -
        $clip_height
    ]) cube([ $wall_twice, $clip_width, $clip_height ]);
};

$hole_numb_x = 9;
$hole_size = 4.95;
$hole_space = ($board_length - ($hole_numb_x * $hole_size)) / $hole_numb_x;

// Position back at origin when you only want to export the top.
// translate([0, 0, 0]) {
translate([ 0, $comp_width + 10, 0 ])
{
    difference()
    {
        // Base for top
        roundedcube(
            [
                $board_length + $wall_twice,
                $comp_width + $wall_twice,
                $top_clearance
            ],
            false,
            1,
            "zmin");

        // Hollow out
        translate([ $wall_thickness, $wall_thickness, $wall_thickness ])
            roundedcube([ $board_length, $comp_width, $top_clearance ],
                        false,
                        1,
                        "zmin");

        // Looping works a bit different in openscad.
        // Predefine some variables to make it easier.
        $x_start = $wall_thickness + ($hole_space / 2);
        $x_end = $board_length - ($hole_space / 2);
        $x_step = $hole_size + $hole_space;

        $y_start = $wall_thickness;
        $y_end = $board_width - ($hole_space / 2);
        $y_offset = ($board_width - (6 * $hole_size) - (5 * $hole_space)) / 2;

        echo($hole_space);

        for ($y = [$y_start + $y_offset:$x_step:$y_end]) {
            for ($x = [$x_start:$x_step:$x_end]) {

                translate([ $x + ($hole_size / 2), $y + ($hole_size / 2), 0 ])
                    rotate([ 0, 0, 45 ])
                        cube([ $hole_size, $hole_size, $wall_twice ]);
            };
        }

        // For some reason can't get this in one loop.
        $half = ($hole_size + $hole_space) / 2;
        for ($y = [$y_start + $half + $y_offset:$x_step:$y_end]) {
            for ($x = [$x_start + $half:$x_step:$x_end]) {

                translate([ $x + ($hole_size / 2), $y + ($hole_size / 2), 0 ])
                    rotate([ 0, 0, 45 ])
                        cube([ $hole_size, $hole_size, $wall_twice ]);
            };
        }

        // Carve text from grid
        translate([
            (($board_length + $wall_twice) / 2) + 11,
            (($board_width + $wall_twice) / 2),
            4
        ]) rotate([ 0, 180, 0 ]) linear_extrude(4)
            text("CO", valign = "baseline", font = "Cascadia Code:style=Bold");
        translate([
            (($board_length + $wall_twice) / 2) - 5,
            (($board_width + $wall_twice) / 2),
            4
        ]) rotate([ 0, 180, 0 ]) linear_extrude(4)
            text("2",
                 valign = "center",
                 font = "Cascadia Code:style=Bold",
                 size = 6);
    }

    // Create clips
    // translate([0, $comp_width + 10, 0]) {
    // same as part 1;
    $height = ($wall_thickness) + $dip_height + ($board_thickness * 2) +
              $comp_height + $port_height;

    $clip_support_thickness = ($wall_thickness * 3) / 4;

    translate([
        $wall_thickness / 2,
        ($comp_width + $wall_twice) / 2 - (($clip_width - $double_spacing) / 2),
        0
    ])
    {
        union()
        {
            translate([ $wall_thickness / 2, 0, 0 ]) cube([
                $clip_support_thickness,
                $clip_width - $double_spacing,
                $clip_wall_offset + $top_clearance +
                $spacing
            ]);
            translate([
                0 - ($wall_thickness - $clip_support_thickness),
                $clip_width - $double_spacing,
                $clip_wall_offset + $top_clearance +
                $spacing
            ]) rotate([ 0, 0, -90 ])
                prism($clip_width - $double_spacing,
                      $wall_thickness + ($wall_thickness / 2),
                      $clip_height);
        }
    }

    translate([
        $board_length,
        ($comp_width + $wall_twice) / 2 - (($clip_width - $double_spacing) / 2),
        0
    ])
    {
        union()
        {
            translate([ $wall_thickness - $clip_support_thickness, 0, 0 ])
                cube([
                    $clip_support_thickness,
                    $clip_width - $double_spacing,
                    $clip_wall_offset + $top_clearance +
                    $spacing
                ]);
            translate([
                $wall_thickness + $clip_support_thickness,
                0,
                $clip_wall_offset + $top_clearance +
                $spacing
            ]) rotate([ 0, 0, 90 ])
                prism($clip_width - $double_spacing,
                      $wall_thickness + ($wall_thickness / 2),
                      $clip_height);
        }
    }
        
    // fill up the holes in the middle
    difference() {
        translate([(($board_length + $wall_twice) / 2), (($board_width + $wall_twice) / 2) - ($hole_size * 2 + $hole_space), 0])
            rotate([ 0, 0, 45 ])
            cube([$hole_size * 3 + $hole_space * 2.5, $hole_size * 3 + $hole_space * 2.5, $wall_thickness]);
        
        // text
        translate([(($board_length + $wall_twice) / 2) + 11, (($board_width + $wall_twice) / 2), 4]) 
            rotate([ 0, 180, 0 ]) 
            linear_extrude(4) 
            text("CO", valign = "baseline", font = "Cascadia Code:style=Bold");

        translate([(($board_length + $wall_twice) / 2) - 5, (($board_width + $wall_twice) / 2), 4]) 
            rotate([ 0, 180, 0 ]) 
            linear_extrude(4) 
            text("2", valign = "center", font = "Cascadia Code:style=Bold", size = 6);
    }
}

/*****************
Imported functions
*****************/

// Source: https://danielupshaw.com/openscad-rounded-corners/
// Higher definition curves
$fs = 0.01;

module roundedcube(size = [1, 1, 1], center = false, radius = 0.5, apply_to = "all") {
	// If single value, convert to [x, y, z] vector
	size = (size[0] == undef) ? [size, size, size] : size;

	translate_min = radius;
	translate_xmax = size[0] - radius;
	translate_ymax = size[1] - radius;
	translate_zmax = size[2] - radius;

	diameter = radius * 2;

	module build_point(type = "sphere", rotate = [0, 0, 0]) {
		if (type == "sphere") {
			sphere(r = radius);
		} else if (type == "cylinder") {
			rotate(a = rotate)
			cylinder(h = diameter, r = radius, center = true);
		}
	}

	obj_translate = (center == false) ?
		[0, 0, 0] : [
			-(size[0] / 2),
			-(size[1] / 2),
			-(size[2] / 2)
		];

	translate(v = obj_translate) {
		hull() {
			for (translate_x = [translate_min, translate_xmax]) {
				x_at = (translate_x == translate_min) ? "min" : "max";
				for (translate_y = [translate_min, translate_ymax]) {
					y_at = (translate_y == translate_min) ? "min" : "max";
					for (translate_z = [translate_min, translate_zmax]) {
						z_at = (translate_z == translate_min) ? "min" : "max";

						translate(v = [translate_x, translate_y, translate_z])
						if (
							(apply_to == "all") ||
							(apply_to == "xmin" && x_at == "min") || (apply_to == "xmax" && x_at == "max") ||
							(apply_to == "ymin" && y_at == "min") || (apply_to == "ymax" && y_at == "max") ||
							(apply_to == "zmin" && z_at == "min") || (apply_to == "zmax" && z_at == "max")
						) {
							build_point("sphere");
						} else {
							rotate = 
								(apply_to == "xmin" || apply_to == "xmax" || apply_to == "x") ? [0, 90, 0] : (
								(apply_to == "ymin" || apply_to == "ymax" || apply_to == "y") ? [90, 90, 0] :
								[0, 0, 0]
							);
							build_point("cylinder", rotate);
						}
					}
				}
			}
		}
	}
}

// Source: https://en.wikibooks.org/wiki/OpenSCAD_User_Manual/Primitive_Solids
module prism(l, w, h){
    polyhedron(
        points=[[0,0,0], [l,0,0], [l,w,0], [0,w,0], [0,w,h], [l,w,h]],
        faces=[[0,1,2,3],[5,4,3,2],[0,4,5,1],[0,3,4],[5,2,1]]
    );
}