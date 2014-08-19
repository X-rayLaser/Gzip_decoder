#include <iostream>
#include <cstdlib>
#include <vector>
#include "Gzip.h"
#include "Huf_tree.h"
#include "Bit_stream.h"
#include "Deflate.h"

int main()
{

	gzip::Gzip_stream gzstr("D:\\EclipseKepler\\eclipse\\myprojects\\GzipDecoder\\Debug\\messagetxt.txt.gz");
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
	catch (std::vector<unsigned char> v){
			std::cout<<v.size();
	}
	catch (int x){
		std::cout<< x;
	}

	system("Pause");
	return 0;
}
