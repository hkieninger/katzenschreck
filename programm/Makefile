CC = g++
FLAGS = -O3 #increases the speed significantly
#FLAGS = -g #debuging

#target for eclipse compilation
all: install

#target install the programm
install: webcam-server interface.html share.cgi
	cp webcam-server /usr/local/bin/webcam-server
	cp interface.html /var/www/html/interface.html
	cp space.js /var/www/html/space.js
	cp color.js /var/www/html/color.js
	cp share.cgi /usr/lib/cgi-bin/share.cgi

#target uninstall the programm
uninstall:
	rm /usr/local/bin/webcam-server
	rm /var/www/html/interface.html
	rm /var/www/html/space.js
	rm /var/www/html/color.js
	rm /usr/lib/cgi-bin/share.cgi

#target compile the webcam-server programm
webcam-server: main.o webcam.o shared_mem.o picture.o base64.o object.o share_manager.o stepper_motor.o servo_stepper.o laser.o move.o
	$(CC) -o $@ $^ -ljpeg -lwiringPi -lm

#target compile the share programm
share.cgi: share.o shared_mem.o cgi.o picture.o base64.o object.o share_manager.o
	$(CC) -o $@ $^ -ljpeg -lm

#target release resources if programm isn't shutdown trough SIGINT
ipcrm:
	ipcrm -S 0x12974465
	ipcrm -M 0x66f197aa

#remove the object files
clean:
	rm -f *.o

share.o: share.cpp shared_mem.h cgi.h
	$(CC) -c $(FLAGS) $<

main.o: main.cpp picture.h webcam.h object.h shared_mem.h stepper_motor.h laser.h move.h servo_stepper.h
	$(CC) -c $(FLAGS) $<

share_manager.o: share_manager.cpp share_manager.h base64.h object.h picture.h shared_mem.h
	$(CC) -c $(FLAGS) $<

cgi.o: cgi.cpp cgi.h
	$(CC) -c $(FLAGS) $<
	
webcam.o: webcam.cpp webcam.h picture.h
	$(CC) -c $(FLAGS) $<

shared_mem.o: shared_mem.cpp shared_mem.h
	$(CC) -c $(FLAGS) $<

picture.o: picture.cpp picture.h
	$(CC) -c $(FLAGS) $<

base64.o: base64.cpp base64.h
	$(CC) -c $(FLAGS) $<

object.o: object.cpp object.h picture.h
	$(CC) -c $(FLAGS) $<
	
stepper_motor.o: stepper_motor.cpp stepper_motor.h
	$(CC) -c $(FLAGS) $<
	
servo_stepper.o: servo_stepper.cpp servo_stepper.h stepper_motor.h
	$(CC) -c $(FLAGS) $<

laser.o: laser.cpp laser.h
	$(CC) -c $(FLAGS) $<

move.o: move.cpp move.h stepper_motor.h servo_stepper.h object.h
	$(CC) -c $(FLAGS) $<
