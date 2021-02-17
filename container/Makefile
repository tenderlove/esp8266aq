OPENSCAD = /Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD

%.stl: case.scad
	$(OPENSCAD) -D'rendering="$(basename $@)"' $< -o $@

all: top.stl bottom.stl

clean:
	rm *.stl
