#include <iostream>
#include <fstream>
using namespace std;
int create_rtp(unsigned long long * buffer, int size, unsigned int seq_nr, unsigned long long timestamp, unsigned long long synch_ident)
{
    unsigned long long temp=0;
    FILE * file;  
    ofstream myWriteFile; 
    temp = 0x80700000; 
    temp += seq_nr;

    cout << hex << temp ;
    buffer[0] = temp;
    buffer[1] = timestamp;
    buffer[2] = synch_ident;
    myWriteFile.open("dupa");
    if(myWriteFile.is_open())
    {
        
	    for (int i =0 ; i <1 ; i++)
	    {
	        myWriteFile << buffer[i] ;	

	    }
    }
    myWriteFile.close();
    return 0;
}

int main(void)
{
    unsigned long long  buffer[10];
    create_rtp(buffer, 2, 2,2,2);

    return 0;
}
