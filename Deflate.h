/*
 * Deflate.h
 *
 *  Created on: 01 авг. 2014 г.
 *      Author: User
 */

#ifndef DEFLATE_H_
#define DEFLATE_H_

#include <cstdlib>
#include <vector>
#include <map>
#include "Bit_stream.h"
#include "Huf_tree.h"
#include <iostream>
#include <boost/filesystem/fstream.hpp>
#include <exception>

namespace deflate
{

class defl_except : public std::exception{
};

class bad_fopen : public defl_except{
};

class bad_mem : public defl_except{
};

class bad_fstate : public defl_except{
};

class bad_blctype : public defl_except{
};

class bad_code : public defl_except{
};

class bad_clen : public defl_except{
};

class bad_repeat : public defl_except{
};

class codes_overflow : public defl_except{
};

class no_eob : public defl_except{
};

class bad_match : public defl_except{
};

class bad_stored : public defl_except{
};



/* min buffer size for memory allocation while writing (in KB)
 * the value 544 = 512 + 32 is choosen because it allows ofstream to write
 * with an optimal amount of bytes (512 KB).
 *
 * NOTE: MIN_BUFFER_SIZE must be greater than 64 + 32 KB to ensure
 * reliable work of obuffer class
 * */
const unsigned int MIN_BUFFER_SIZE = 544 ;
const unsigned int CMP_RATIO = 3;		   //expected compression ratio of a file
const unsigned WND_SZ = 32768;			   //window size in bytes

const int END_OF_BLOCK = 256 ;

struct extra_bits{
	int bits;
	int min_val;
};

class obuffer {
	boost::filesystem::ofstream out;
	std::vector<unsigned char> buf;
	size_t max_size;
	size_t data_size;

	size_t memlimit();
	void flush_buf();
	void write_raw(const unsigned char* data, int size);
public:
	obuffer(boost::filesystem::path fname, size_t finp_size);

	void write(const std::vector<unsigned char>& data)
	{
		write_raw(&data[0], data.size()) ;
	}

	void put_byte(const unsigned char& byte);
	void copy(int len, int dist);
	void close();
};


class Deflate_stream{
	std::map<int, struct extra_bits> lengths ;
	std::map<int, struct extra_bits> distances ;
	btstream::Bit_stream btstr;
	obuffer out_buffer;
	bool last_block;

	int get_value(const tree::Huf_tree& htr);
	tree::Huf_tree get_lenghts_tree(int count);
	std::vector<tree::pair> decode_rle(const tree::Huf_tree& htr, int nlit, int ndist);

	tree::Huf_tree init_lentree();
	tree::Huf_tree init_disttree();
	void uncompressed();
	void fixed_huf();
	void dynamic_huf();
public:
	Deflate_stream(boost::filesystem::path fin, boost::filesystem::path fout,
			int offset, size_t input_sz);
	void decode_block();
	bool last_blck() { return last_block; }
	void close() { btstr.close(); out_buffer.close(); }
};



}

#endif /* DEFLATE_H_ */
