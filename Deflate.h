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

const unsigned int OUTBUF_SZ = 100000;
const unsigned WND_SZ = 32768;

struct extra_bits{
	int bits;
	int min_val;
};

class obuffer {
	std::ofstream out;
	std::vector<unsigned char> buf;
public:
	obuffer(const char* fname) : out()
	{
		out.open(fname,std::ios::out | std::ios::binary);
		buf.reserve(OUTBUF_SZ);
	}
	void put_byte(unsigned char byte);
	void write(const std::vector<unsigned char>& data);
	void close();
};


class wnd32k{
	std::vector<unsigned char> wnd;
public:
	wnd32k(){ wnd.reserve(2*WND_SZ);}
	void put_byte(unsigned char byte);
	void append(const std::vector<unsigned char>& v );
	std::vector<unsigned char> retrieve(int len, int dist);
};


class Deflate_stream{
	std::map<int, struct extra_bits> lengths ;
	std::map<int, struct extra_bits> distances ;
	Bit_stream btstr;
	obuffer out_buffer;
	wnd32k w;
	bool last_block;

	int get_value(const tree::Huf_tree& htr);
	void copy_bytes(int len, int dist);
	tree::Huf_tree get_lenghts_tree(int count);
	std::vector<tree::pair> decode_rle(const tree::Huf_tree& htr, int nlit, int ndist);

	tree::Huf_tree init_fixtree();
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
