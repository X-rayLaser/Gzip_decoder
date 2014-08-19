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

class bad_fstate{
};

class bad_blctype{
};

class bad_code{
};

class bad_clen{
};

class bad_chksum{
};

class bad_codetbl{
};

const unsigned int MEMORY_LIMIT = 100000000 ;
const unsigned int CMP_RATIO = 3;
const unsigned WND_SZ = 32768;

struct extra_bits{
	int bits;
	int min_val;
};

class obuffer {
	std::ofstream out;
	std::vector<unsigned char> buf;
	unsigned int max_size;
	unsigned int data_size;

	void flush_buf();
	void write_raw(const unsigned char* data, int size);
public:
	obuffer(const char* fname, int finp_size) : out()
	{
		max_size = finp_size * CMP_RATIO ;
		if (max_size > MEMORY_LIMIT)
			max_size = MEMORY_LIMIT;
		else if (max_size < WND_SZ)
			max_size = WND_SZ * 2 ;

		data_size = 0;
		out.open(fname, std::ios::out | std::ios::binary);
		buf.resize(max_size) ;
	}

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
	Bit_stream btstr;
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
	Deflate_stream(const char* fin, const char* fout, int offset);
	void decode_block();
	bool last_blck() { return last_block; }
	void close() { btstr.close(); out_buffer.close(); }
};



#endif /* DEFLATE_H_ */
