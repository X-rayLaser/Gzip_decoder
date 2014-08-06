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

const int BUF_SZ = 100000;

class buf_excpt{
};

class bad_init: public buf_excpt{
};

class bad_refill: public buf_excpt{
};

class ibuffer {
	std::ifstream in;
	unsigned char* buf;
	int cur_byte;
	const int buf_size;
	int data_size;

	void refill();
public:
	ibuffer(const char* fname, int offset );
	unsigned char get_byte();
	bool eof()
	{
		return data_size != buf_size && cur_byte == data_size ;
	}

	~ibuffer()
	{
		in.close();
		delete [] buf;
	}
};

class Bit_stream{
	ibuffer buffer ;
	unsigned char cur_byte;
	unsigned char bit_pos;
public:
	Bit_stream(const char* fname, int offset);
	bool get_bit();
	int read_reverse(int bit_count);
	void skip_bits();
	unsigned char get_byte();
	unsigned short get_wordLE();
	bool end_of_stream();
};


#endif /* BIT_STREAM_H_ */
