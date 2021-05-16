OPENSCAD = /Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD

%.stl: case.scad
	$(OPENSCAD) -D'rendering="$(basename $@)"' $< -o $@

all: top.stl bottom.stl top-with-lid.stl bottom-with-lid.stl

clean:
	rm -f *.stl
