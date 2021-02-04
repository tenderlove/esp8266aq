PM25_X = 38.3;
PM25_Y = 50.1;
PM25_Z = 21.3;
WALL_SIZE = 1;

TAB_X = 5.75;
TAB_Y = 5.75;

PCB_SPACER = 2;

PCB_THICKNESS = 1.8;

BRIM = TAB_X;

module leftCutout() {
  translate([-WALL_SIZE, BRIM, 0])
  linear_extrude(PM25_Z)
    square(size = [WALL_SIZE, PM25_Y - (2 * BRIM)]);
}

module bottomCutout() {
  translate([-WALL_SIZE, BRIM, -WALL_SIZE])
    linear_extrude(WALL_SIZE)
    square(size = [PM25_X + (WALL_SIZE * 2), PM25_Y - (2 * BRIM)]);
}

module pcbCutout() {
  translate([TAB_X, 0, PM25_Z])
    linear_extrude(PCB_SPACER + PCB_THICKNESS + WALL_SIZE)
    square(size = [PM25_X - (TAB_X * 2), PM25_Y]);

  translate([TAB_X, -WALL_SIZE, PM25_Z + PCB_SPACER + PCB_THICKNESS])
    linear_extrude(WALL_SIZE)
    square(size = [PM25_X - (TAB_X * 2), PM25_Y + (WALL_SIZE * 2)]);

  translate([0, TAB_Y, PM25_Z])
    linear_extrude(PCB_SPACER + PCB_THICKNESS + WALL_SIZE)
    square(size = [PM25_X, PM25_Y - (TAB_Y * 2)]);

  translate([-WALL_SIZE, TAB_Y, PM25_Z + PCB_SPACER + PCB_THICKNESS])
    linear_extrude(WALL_SIZE)
    square(size = [PM25_X + (WALL_SIZE * 2), PM25_Y - (TAB_Y * 2)]);

  translate([0, 0, PM25_Z + PCB_SPACER])
    linear_extrude(PCB_THICKNESS)
    square(size = [PM25_X, PM25_Y]);
}

module pmcutout() {
  port_d = 2.45;
  fan_r = 9;

  jst_height = 4.9;
  jst_len = 17;
  translate([-WALL_SIZE, PM25_Y - jst_len - 5.5, (PM25_Z / 2) + jst_height / 2])
  rotate([0, 90, 0])
    color("blue")
    linear_extrude(WALL_SIZE)
    square(size = [jst_height, jst_len]);

  translate([PM25_X, 35.9, PM25_Z / 2])
  rotate([0, 90, 0])
    linear_extrude(WALL_SIZE)
    circle(r = fan_r);

  translate([PM25_X, 6 - (port_d / 2), PM25_Z - (4.2 - port_d)])
    rotate([0, 90, 0])
    color("green")
    linear_extrude(WALL_SIZE)
    square(size = [port_d, 12.0 + port_d]);

  linear_extrude(PM25_Z)
    square(size = [PM25_X, PM25_Y]);
}

module pmcase() {
  difference() {
    translate([-WALL_SIZE, -WALL_SIZE, -WALL_SIZE])
      linear_extrude(PM25_Z + (WALL_SIZE * 2) + PCB_SPACER + PCB_THICKNESS)
      square(size = [PM25_X + (WALL_SIZE * 2), PM25_Y + (WALL_SIZE * 2)]);
    pmcutout();
    pcbCutout();
    bottomCutout();
    leftCutout();
  }
}

pmcase();
//pmcutout();
