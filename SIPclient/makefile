aal:	main2.o md5.o 2client.o messages.o parser.o
	g++ main2.o md5.o 2client.o messages.o parser.o -L/usr/local/lib -losip2 -losipparser2 -lpthread -lmd5 -o sip
main2.o:	main2.cpp
	g++ -c main2.cpp -o main2.o -losip2 -lpthread 
md5.o:	md5.c
	g++ -c md5.c -o md5.o
2client.o:
	g++ -c 2client.cpp -o 2client.o -lpthread
messages.o:	messages.cpp messages.h
	g++ -c messages.cpp -o messages.o
parser.o:	parser.cpp parser.h
	g++ -c parser.cpp -o parser.o
clean:
	rm *.o
