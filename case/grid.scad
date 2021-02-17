function round_down(n) = n - (n % 1);

module Grid(x, y, inner_diameter = 5, grid_buffer = 2, center = false) {
  diameter = inner_diameter + grid_buffer;
  radius   = diameter / 2;
  x_n = round_down(x / diameter);
  y_n = round_down(y / diameter);
  $fn = 8;

  y_diff = y - (y_n * diameter);
  x_diff = x - (x_n * diameter);

  move = center ?
    [radius - (diameter * x_n) / 2, radius - (diameter * y_n) / 2, 0]
    :
    [radius + (x_diff / 2), radius + (y_diff / 2), 0];

  translate(move)
    for (j = [0 : (y_n - 1)]) {
      for (i = [0 : (x_n - 1)]) {
        translate([i * diameter, j * diameter, 0])
          circle(d = inner_diameter);
      }
  }
}
