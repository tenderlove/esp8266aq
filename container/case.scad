PM25_X = 38.3;
PM25_Y = 50.1;
PM25_Z = 21.3;
WALL_SIZE = 1.5;
TAB_X = 5.75;
TAB_Y = 5.75;
PCB_SPACER = 2;
PCB_THICKNESS = 1.8;
BRIM = TAB_X;
CABLE_BOX_X = 13;
BIG_VENT_CENTER = 35.9;

// JST cable cutout
JST_DISTANCE_FROM_EDGE = 9.3;
JST_WIDTH = 11;
CABLE_SPACE = 2;

module leftCutout() {
  translate([-WALL_SIZE, BRIM, 0])
    linear_extrude(PM25_Z)
    square(size = [WALL_SIZE, PM25_Y - (2 * BRIM)]);

  translate([-WALL_SIZE, JST_DISTANCE_FROM_EDGE, 0])
    linear_extrude(PM25_Z + PCB_THICKNESS + PCB_SPACER)
    square(size = [WALL_SIZE, JST_WIDTH]);
}

module rightCutout() {
  translate([PM25_X, BRIM, -WALL_SIZE])
    linear_extrude(PM25_Z + PCB_THICKNESS + PCB_SPACER + (WALL_SIZE * 2))
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

  translate([PM25_X, BIG_VENT_CENTER, PM25_Z / 2])
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
  translate([CABLE_BOX_X + WALL_SIZE, 0, 0]) {
    translate([WALL_SIZE, WALL_SIZE, WALL_SIZE])
      difference() {
        translate([-WALL_SIZE, -WALL_SIZE, -WALL_SIZE])
          linear_extrude(PM25_Z + (WALL_SIZE * 2) + PCB_SPACER + PCB_THICKNESS)
          square(size = [PM25_X + (WALL_SIZE * 2), PM25_Y + (WALL_SIZE * 2)]);
        pmcutout();
        pcbCutout();
        //bottomCutout();
        leftCutout();
        rightCutout();
      }
    translate([-(CABLE_BOX_X + WALL_SIZE), 0, 0])
      cablecase();
  }
}

module cablecaseCutout() {
  height = PM25_Z + PCB_SPACER + PCB_THICKNESS - WALL_SIZE;
  linear_extrude(height)
    square(size = [CABLE_BOX_X, PM25_Y]);

  translate([CABLE_BOX_X - CABLE_SPACE, JST_DISTANCE_FROM_EDGE, height])
    linear_extrude(WALL_SIZE)
    square(size = [CABLE_SPACE, JST_WIDTH]);
}

module cablecase() {
  translate([WALL_SIZE, WALL_SIZE, WALL_SIZE])
  difference() {
    translate([-WALL_SIZE, -WALL_SIZE, -WALL_SIZE])
      linear_extrude(PM25_Z + PCB_SPACER + PCB_THICKNESS + WALL_SIZE)
      square(size = [CABLE_BOX_X + WALL_SIZE, PM25_Y + (WALL_SIZE * 2)]);
    cablecaseCutout();
  }
}

module Bottom() {
  difference() {
    pmcase();

    translate([CABLE_BOX_X + WALL_SIZE, (BIG_VENT_CENTER + WALL_SIZE), 0])
      color("blue")
      linear_extrude(PM25_Z + WALL_SIZE * 2 + PCB_SPACER + PCB_THICKNESS)
      square(size = [PM25_X + (WALL_SIZE * 2), (PM25_Y - BIG_VENT_CENTER + WALL_SIZE)]);

    translate([0, JST_DISTANCE_FROM_EDGE + WALL_SIZE + JST_WIDTH, 0])
      linear_extrude(PM25_Z + WALL_SIZE * 2 + PCB_SPACER + PCB_THICKNESS)
      square(size = [CABLE_BOX_X + (WALL_SIZE * 2), PM25_Y + (WALL_SIZE * 2)]);

  }
}

module TopSlice() {
    translate([CABLE_BOX_X + (WALL_SIZE * 2), 0, 0])
      color("blue")
      linear_extrude(PM25_Z + WALL_SIZE * 2 + PCB_SPACER + PCB_THICKNESS)
      square(size = [PM25_X + WALL_SIZE, BIG_VENT_CENTER + WALL_SIZE]);

      linear_extrude(PM25_Z + WALL_SIZE * 2 + PCB_SPACER + PCB_THICKNESS)
      square(size = [CABLE_BOX_X + (WALL_SIZE * 2), WALL_SIZE + JST_DISTANCE_FROM_EDGE + JST_WIDTH]);

}

module Top() {
  difference() {
    pmcase();
    TopSlice();
  }
}

//pmcase();
//Top();
rotate([90, 0, 0])
Bottom();