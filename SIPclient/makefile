aal:	main.o md5.o messages.o parser.o rtpclient.o rtp.o
	g++ main.o md5.o messages.o parser.o rtpclient.o rtp.o -L/usr/local/lib -losip2 -losipparser2 -lpthread -lmd5 -o sip
main.o:	main.cpp
	g++ -c main.cpp -o main.o -losip2 -lpthread 
md5.o:	md5.c
	g++ -c md5.c -o md5.o
rtpclient.o: rtpclient.cpp rtpclient.h
	g++ -c rtpclient.cpp -o rtpclient.o 
rtp.o:	rtp.cpp rtp.h
	g++ -c rtp.cpp -o rtp.o
messages.o:	messages.cpp messages.h
	g++ -c messages.cpp -o messages.o
parser.o:	parser.cpp parser.h
	g++ -c parser.cpp -o parser.o
clean:
	rm *.o
