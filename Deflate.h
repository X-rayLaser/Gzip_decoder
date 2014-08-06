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
#include "Huf_tree.h"

using namespace std;

struct extra_bits{
	int bits;
	int min_val;
};




class Deflate_stream{
	const std::map<int, struct extra_bits> lengths ;
	const std::map<int, struct extra_bits> distances ;
	Bit_stream btstr;
	vector<unsigned char> out;
	bool last_block;

	int get_value(const tree::Huf_tree& htr);
	void copy_bytes(int len, int dist);
	tree::Huf_tree get_lenghts_tree(int count);
	tree::Huf_tree decode_rle(const tree::Huf_tree& htr, int count);

	tree::Huf_tree init_fixtree();
	void uncompressed();
	void fixed_huf();
	void dynamic_huf();
public:
	Deflate_stream(const char* fname, int offset);
	void decode_block();
	bool eos();
	vector<char> stream();
};



#endif /* DEFLATE_H_ */
