
as7265x: ./src/as7265x.c
	gcc -o $@ -I ./src $^

run: as7265x
	sudo ./as7265x
