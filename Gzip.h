/*
 * Gzip.h
 *
 *  Created on: 08 авг. 2014 г.
 *      Author: User
 */

#ifndef GZIP_H_
#define GZIP_H_

#include <vector>
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <exception>
#include <stdint.h>

namespace gzip {

class gz_except : public std::exception{
};

class bad_format : public gz_except{
};

class bad_id : public gz_except{
};

class bad_fopen : public gz_except{
};

const unsigned char ID1 = 31;
const unsigned char ID2 = 139;
const unsigned char CM  = 8;

const int BUFFER_SIZE = 512 ;
// (in KB) for reading an output file and calculating its crc32 check sum

const unsigned char FTEXT 	 = 1;
const unsigned char FHCRC 	 = 2;
const unsigned char FEXTRA   = 4;
const unsigned char FNAME  	 = 8;
const unsigned char FCOMMENT = 16;
const unsigned char RES1	 = 32;
const unsigned char RES2	 = 64;
const unsigned char RES3	 = 128;

class Extra_field{
public:
	unsigned char id1;
	unsigned char id2;
	std::vector<unsigned char> data;

	Extra_field(unsigned char si1, unsigned char si2,
			const std::vector<unsigned char>& subf_data):
				id1(si1), id2(si2), data(subf_data){}
};

typedef struct {
	unsigned char id1;
	unsigned char id2;
	unsigned char cm;
	unsigned char flg;
	uint32_t  	  mtime;
	unsigned char xfl;
	unsigned char os;
	uint32_t 	  crc32;
	uint32_t	  isize;
} base_fields;


class Gzip_stream   {
	boost::filesystem::ifstream  in;
	boost::filesystem::path      archive_name;
	unsigned long  offset;

	base_fields    fields;

	bool 		   is_txt;
	Extra_field*   extra_f;
	std::string    fname;
	std::string    fcomment;
	uint16_t	   crc16;

	void read_flds();
	void read_fextra();
	void read_fname();
	void read_fcomment();
	boost::filesystem::path output_fname();
public:
	Gzip_stream(boost::filesystem::path&  fin);
	void decode();
	bool is_correct();
	bool is_text() { return is_txt; }
	std::string   get_fname()   { return fname;        }
	std::string   get_comment() { return fcomment;     }
	unsigned int  get_mtime()   { return fields.mtime; }
	unsigned char get_os()		{ return fields.os;	   }

	bool get_xfield(Extra_field& xfield);

	~Gzip_stream()
	{
		if (extra_f)
			delete extra_f;
	}
};


}

#endif /* GZIP_H_ */
