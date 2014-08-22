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
#include <fstream>

/*
void setFontSize(int x, int y)
{
	PCONSOLE_FONT_INFOEX lpConsoleCurrentFontEx = new CONSOLE_FONT_INFOEX();
	lpConsoleCurrentFontEx->cbSize = sizeof(CONSOLE_FONT_INFOEX);
	GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, lpConsoleCurrentFontEx);
	lpConsoleCurrentFontEx->dwFontSize.X = x;
	lpConsoleCurrentFontEx->dwFontSize.Y = y;
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, lpConsoleCurrentFontEx);
}
*/

int main()
{

    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

	std::cout<<"File name: ";
	std:: string fn;
	std::getline(std::cin, fn);

	boost::filesystem::path p = fn;

	gzip::Gzip_stream gzstr(p);
	try{
		gzstr.decode();
		if (gzstr.is_correct() )
			std::cout<<"The file was successfully unpacked!\n";
		else
			std::cout<<"The file wasn't unpacked correctly!\n";
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
