#include "Bit_stream.h"

ibuffer::ibuffer(const char* fname, int offset): in(fname), buf(INBUF_SZ)
{
	if (!in.is_open())
		throw bad_init;

	in.seekg(offset);

	if (!in.good())
		throw bad_init;

	refill();
}

void ibuffer::refill()
{
	if (!in.good() )
		throw bad_refill;

	in.read((char*) &buf[0], sizeof(char)*buf.size());
	data_size = in.gcount();

	if (!in.good() && !in.eof())
		throw bad_refill;

	byte_pos=0;
}

unsigned char ibuffer::get_byte()
{
	if (byte_pos < data_size){
		unsigned char byte = buf[byte_pos];
		byte_pos++;
		return  byte;
	}
	else {
		refill();
		unsigned char byte = buf[byte_pos];
		byte_pos++;
		return  byte;
	}
}


Bit_stream::Bit_stream(const char* fname, int offset): in_buffer(fname, offset)
{
	cur_byte = in_buffer.get_byte();

	bit_pos = 0;

}


bool Bit_stream::get_bit()
{
	static unsigned char bits[8] = {1, 2, 4, 8, 16, 32, 64, 128};

	if (bit_pos == 8){
		cur_byte = in_buffer.get_byte();
		bit_pos = 0;
	}


	unsigned char bit = cur_byte & bits[bit_pos];
	if (bit_pos != 0)
		bit = bit >> bit_pos;

	bit_pos++;

	return bit == 1;
}

/* read a given number of bits in MSB order */
int Bit_stream::read_reverse(int bit_count)
{
	int res = 0;
	for (int i = 0; i < bit_count; i++)
		if (i == 0)
			res = res | get_bit() ;
		else
			res = res | ( (get_bit()) << i);

	return res;
}

/* get current byte, extract a next byte from a buffer */
unsigned char Bit_stream::get_byte()
{
	unsigned char byte = cur_byte;

	if (!in_buffer.eof()){
		cur_byte = in_buffer.get_byte();
		bit_pos = 0;
	}
	else
		bit_pos = 8;

	return byte;
}

/* get word written as a little endian */
unsigned short Bit_stream::get_wordLE()
{
	/*
	 * Questionable
	 * Perhaps this is not a safe way to use a type
	 * unsigned short
	 */
	unsigned short byte0 = get_byte();
	unsigned short byte1 = get_byte();

	return byte0 + byte1*256 ;
}

/* skip bits in partially used byte till the
 * bytes boundary
 */
void Bit_stream::skip_bits()
{
	if (bit_pos !=0){
		cur_byte = in_buffer.get_byte();
		bit_pos = 0;
	}
}


