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

	gzip::Gzip_stream gzstr("D:\\EclipseKepler\\eclipse\\myprojects\\GzipDecoder\\Debug\\biorne.gz");
	try{
		gzstr.decode();
	}
	catch(tree::bad_code)
	{
		cout<<"tree bad code";
	}
	catch(buf_excpt&){
		cout<<"buf_excpt";
	}
	catch (bad_clen){
		cout<<"bad_clen";
	}
	catch (bad_code){
		cout<<"bad_code\n";
	}
	catch (bad_blctype){
		cout<<"bad_blctype\n";
	}
	catch (bad_chksum){
		cout<<"bad_chksum  \n";
	}
	catch (vector<tree::pair> v){
		for (int i=0; i<v.size(); i++)
			cout<<v[i].value<<"   "<<v[i].length<<'\n';
			cout<<"other problem\n";
	}
	catch (int x){
		cout<< x;
	}

	system("Pause");
	return 0;
}
