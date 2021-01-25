RUBY = ruby
PLATFORMIO = platformio

compile:
	platformio run

flash:
	$(RUBY) utils/prog.rb
	platformio run -t upload
