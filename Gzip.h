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
#include <fstream>

namespace gzip {

class bad_format{
};

class bad_id{
};

class bad_fstate{
};

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
	unsigned int  mtime;
	unsigned char xfl;
	unsigned char os;
	unsigned long crc32;
	unsigned long isize;
} base_fields;


class Gzip_stream   {
	std::ifstream  in;
	std::string    archive_name;
	unsigned long  offset;

	base_fields    fields;

	bool 		   is_txt;
	Extra_field*   extra_f;
	std::string    fname;
	std::string    fcomment;
	unsigned short crc16;

	void read_flds();
	void read_fextra();
	void read_fname();
	void read_fcomment();
	std::string output_fname();
public:
	Gzip_stream(const std::string fin);
	void decode();

	bool is_text() { return is_txt; }
	std::string   get_fname()   { return fname;        }
	std::string   get_comment() { return fcomment;     }
	unsigned int  get_mtime()   { return fields.mtime; }
	unsigned char get_os()		{ return fields.os;	   }

	inline bool get_xfield(Extra_field& xfield);

	~Gzip_stream()
	{
		if (extra_f)
			delete extra_f;
	}
};


}

#endif /* GZIP_H_ */
