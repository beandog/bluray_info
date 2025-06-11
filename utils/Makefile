all:
	gcc -o bluray_id bluray_id.c `pkg-config --libs --cflags libbluray` 

clean:
	rm -v bluray_id

install:
	sudo cp -v bluray_id /usr/local/bin
