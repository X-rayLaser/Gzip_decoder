#include <iostream>
#include <cstdlib>
#include <string>
#include "Gzip.h"
#include "Huf_tree.h"
#include "Bit_stream.h"
#include "Deflate.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <Windows.h>
#include <typeinfo>

namespace {
	boost::filesystem::path get_fname();
	////exception handlers
	void gz_handler(gzip::gz_except& ob);
	void defl_handler(deflate::defl_except& ob);
	void btstr_handler(btstream::buf_except& ob);
	void tree_handler(tree::tree_except& ob);

	/* get a name of a file to be unpacked from the user */
	boost::filesystem::path get_fname()
	{
		bool success = false;
		boost::filesystem::path p;
		do{
		std::cout<<"File name: ";
		std:: string fn;
		std::getline(std::cin, fn);

		p = fn;
		if (!boost::filesystem::exists(p))
			std::cout<<"Invalid file name. Please, try again\n";
		else if (!boost::filesystem::is_regular_file(p))
			std::cout<<"Invalid file name. Please, try again\n";
		else
			success = true ;
		} while (! success);

		return p;
	}

	void gz_handler(gzip::gz_except& ob)
	{
		if ( typeid(ob) == ( typeid(gzip::bad_fopen)) )
			std::cout << "The file can't be open to read. Try to close any program which uses it\n";
		else if ( typeid(ob) == (typeid(gzip::bad_id)) )
			std::cout << "Invalid file ID. It's probably not a gzip file\n";
		else if ( typeid(ob) == (typeid(gzip::bad_format)) )
			std::cout << "Invalid file header\n";
	}


	void defl_handler(deflate::defl_except& ob)
	{
		if ( typeid(ob) == (typeid(deflate::bad_fopen)) )
				std::cout << "Output file can't be open. Try to close any program which uses it\n";
		else if ( typeid(ob) == (typeid(deflate::bad_mem)) )
			std::cout << "Not enough memory for decompression\n";
		else if ( typeid(ob) == (typeid(deflate::bad_fstate)) )
			std::cout << "Error. Failed to write into an output file\n";
		else if ( typeid(ob) == (typeid(deflate::bad_blctype)) )
			std::cout << "Error. Invalid type of a block\n";
		else if ( typeid(ob) == (typeid(deflate::bad_match)) )
			std::cout << "Error. Invalid length of a stored block\n";
		else if ( typeid(ob) == (typeid(deflate::bad_stored)) )
			std::cout << "Error. Failed to read in a stored data\n";
		else if ( typeid(ob) == (typeid(deflate::bad_code)) )
			std::cout<< "Error. Invalid literal/length or back distance code\n";
		else if ( typeid(ob) == (typeid(deflate::bad_clen)) )
			std::cout << "Error. Negative length of a code\n";
		else if ( typeid(ob) == (typeid(deflate::bad_repeat)) )
			std::cout<< "Error. Got a repeat code without any previous codes \n";
		else if ( typeid(ob) == (typeid(deflate::codes_overflow)) )
			std::cout<< "Error. More codes then it was specified in a block header\n";
		else if ( typeid(ob) == (typeid(deflate::no_eob)) )
			std::cout<< "Error. Missing an end of a block code in a code table\n";

	}

	void btstr_handler(btstream::buf_except& ob)
	{
		 if ( typeid(ob) == (typeid(btstream::bad_open)) )
			 std::cout<< "Error. Failed to create an input buffer.\n";
		 else  if ( typeid(ob) ==  (typeid(btstream::bad_init)) )
			std::cout<< "Error. Failed to initiate an input buffer.\n";
		 else  if ( typeid(ob) == (typeid(btstream::bad_refill)) )
			std::cout<< "Error. Failed to refill an input buffer.\n";

	}

	void tree_handler(tree::tree_except& ob)
	{
		if ( typeid(ob) == (typeid(tree::empty_tree)) )
			std::cout<< "Error. Constructing a binary tree with no elements in it.\n";
		else if ( typeid(ob) == (typeid(tree::bad_tree)) )
			std::cout<< "Error. Invalid binary tree has been constructed.\n";
		else if ( typeid(ob) == ( typeid( tree::bad_code)) )
			std::cout<< "Error. Noticed a try to access a non-existing tree node.\n";
	}


}



int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

	boost::filesystem::path p = get_fname();

	try{
		gzip::Gzip_stream gzstr(p);
		gzstr.decode();
		if (gzstr.is_correct() )
			std::cout<<"The file was successfully unpacked!\n";
		else
			std::cout<<"The file wasn't unpacked correctly!\n";
	}
	catch(tree::tree_except& ex){
		tree_handler(ex);
	}
	catch(btstream::buf_except& ex){
		btstr_handler(ex);
	}
	catch (deflate::defl_except& ex){
		defl_handler(ex);
	}
	catch(gzip::gz_except& ex){
		gz_handler(ex);
	}
	catch(...){
		std::cout << "Sorry.:( Some problem has occurred during the decompression\n";
	}
	system("Pause");

	return 0;
}
