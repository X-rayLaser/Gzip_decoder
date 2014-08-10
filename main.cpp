#include <iostream>
#include <cstdlib>
#include <vector>
#include "Gzip.h"
#include "Huf_tree.h"
#include "Bit_stream.h"
#include "Deflate.h"
using namespace std;



int main()
{
	/*
	obuffer out("D:\\EclipseKepler\\eclipse\\myprojects\\GzipDecoder\\Debuggurka.txt");
	ibuffer btstr("D:\\IT\\messagetxt.txt",0);

    unsigned char ch;
	while (! btstr.eof() ){
		vector<unsigned char> v;
		v.reserve(1000);
		for (int i=0; i<1000 && !btstr.eof(); i++)
			v.push_back(btstr.get_byte()) ;

		out.write(v);
	}
	out.close();
*/

	Gzip_stream gzstr("D:\\EclipseKepler\\eclipse\\myprojects\\GzipDecoder\\Debug\\joel.djvu.gz");
	try{
	gzstr.decode();
	}
	catch ( unsigned  int x){
			cout<<x;
	}

	system("Pause");
	return 0;
}
