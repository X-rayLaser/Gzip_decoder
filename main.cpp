#include <iostream>
#include <cstdlib>
#include <vector>
#include "Gzip.h"
#include "Huf_tree.h"
#include "Bit_stream.h"
#include "Deflate.h"
//using namespace std;



int main()
{



	gzip::Gzip_stream gzstr("D:\\EclipseKepler\\eclipse\\myprojects\\GzipDecoder\\Debug\\burk.djvu.gz");
	try{
		gzstr.decode();
	}
	catch(tree::bad_code)
	{
		std::cout<<"tree bad code";
	}
	catch(buf_excpt&){
		std::cout<<"buf_excpt";
	}
	catch (bad_clen){
		std::cout<<"bad_clen";
	}
	catch (bad_code){
		std::cout<<"bad_code\n";
	}
	catch (bad_blctype){
		std::cout<<"bad_blctype\n";
	}
	catch (bad_chksum){
		std::cout<<"bad_chksum  \n";
	}
	catch (std::vector<tree::pair> v){

		for (int i=0; i<v.size(); i++)
			std::cout<< v[i].value<<" "<<v[i].length<<'\n';
	}
	catch (int x){
		std::cout<< x;
	}

	system("Pause");
	return 0;
}
