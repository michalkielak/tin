aal:	main2.o md5.o
	g++ main2.o md5.o -losip2 -lpthread -lmd5 -o sip
main2.o:	main2.cpp messages.cpp parser.cpp
	g++ -c main2.cpp -o main2.o 
md5.o:	md5.c
	g++ -c md5.c -o md5.o
clean:
	rm *.o
