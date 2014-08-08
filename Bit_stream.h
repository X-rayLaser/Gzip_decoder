/*
 * Bit_stream.h
 *
 *  Created on: 06 авг. 2014 г.
 *      Author: User
 */

#ifndef BIT_STREAM_H_
#define BIT_STREAM_H_

#include <iostream>
#include <fstream>
#include <vector>

const int INBUF_SZ = 100000;

class buf_excpt{
};

class bad_init: public buf_excpt{
};

class bad_refill: public buf_excpt{
};

class ibuffer {
	std::ifstream in;
	std::vector<unsigned char> buf;
	int byte_pos;
	int data_size;
	void refill();
public:
	ibuffer(const char* fname, int offset );
	unsigned char get_byte();

	bool eof()
	{
		return in.eof() && byte_pos == data_size ;
	}

	~ibuffer(){ in.close(); }
};

class Bit_stream{
	ibuffer in_buffer ;
	unsigned char cur_byte;
	unsigned char bit_pos;   //current bit in a byte
public:
	Bit_stream(const char* fname, int offset);
	bool get_bit();
	int read_reverse(int bit_count);
	void skip_bits();
	unsigned short get_wordLE();  //get little endian word
	unsigned char get_byte();
	bool end_of_stream() 		 { return in_buffer.eof(); }
	bool end_of_bits()
	{
		return end_of_stream() && (bit_pos == 8);
	}
};


#endif /* BIT_STREAM_H_ */
