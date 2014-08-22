/*
 * Bit_stream.h
 *
 *  Created on: 06 авг. 2014 г.
 *      Author: User
 */

#ifndef BIT_STREAM_H_
#define BIT_STREAM_H_

#include <iostream>
#include <boost/filesystem/fstream.hpp>
#include <vector>


const int INBUF_SZ = 512; //in KB

class buf_excpt{
};

class bad_init: public buf_excpt{
};

class bad_refill: public buf_excpt{
};

class ibuffer {
	boost::filesystem::ifstream in;
	std::vector<unsigned char> buf;
	std::vector<unsigned char>::const_iterator pbuf;
	std::vector<unsigned char>::const_iterator buf_end;
	bool is_end;

	void refill();
public:
	ibuffer(boost::filesystem::path& fname, int offset );
	unsigned char get_byte()
	{
		if (pbuf == buf_end)
			refill();

		return  *pbuf++;
	}

	int read(std::vector<unsigned char>& dest, int count);
	bool eof()
	{
		return is_end ;
	}

	void close(){ in.close(); }
};

class Bit_stream{
	ibuffer in_buffer ;
	unsigned char cur_byte;
	unsigned char bit_pos;   //current bit in a byte
public:
	Bit_stream(boost::filesystem::path& fname, int offset );
	bool get_bit();
	int read_reverse(int bit_count);
	void skip_bits();
	unsigned short get_wordLE();  //get little endian word
	unsigned char get_byte();
	int copy(std::vector<unsigned char>& dest, int count)
	{
		if (count == 0)
			return 0;

		dest.push_back(cur_byte);
		count--;

		int byte_count = in_buffer.read(dest, count) ;
		cur_byte = in_buffer.get_byte();
		bit_pos = 0;
		return  byte_count + 1;
	}

	bool end_of_stream() 		 { return in_buffer.eof(); }
	bool end_of_bits()
	{
		return end_of_stream() && (bit_pos == 8);
	}
	void close() { in_buffer.close(); }
};

#endif /* BIT_STREAM_H_ */
