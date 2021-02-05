OPENSCAD = openscad

%.stl: case.scad
	$(OPENSCAD) -D'rendering="$(basename $@)"' $< -o $@

all: top.stl bottom.stl
