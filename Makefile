all: alsa-ctl-rename2 alsa-ctl-rename1
	@echo Done

clean:
	rm -f alsa-ctl-rename2 alsa-ctl-rename1 

alsa-ctl-rename2: alsa-ctl-rename2.c
	gcc -g alsa-ctl-rename2.c -o alsa-ctl-rename2 -lasound

alsa-ctl-rename1: alsa-ctl-rename1.c
	gcc -g alsa-ctl-rename1.c -o alsa-ctl-rename1 -lasound
