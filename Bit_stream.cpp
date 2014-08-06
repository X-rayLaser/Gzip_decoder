#include "Bit_stream.h"

ibuffer::ibuffer(const char* fname, int offset): in(fname), buf_size(BUF_SZ)
{
	if (!in.is_open())
		throw bad_init;

	in.seekg(offset);

	if (!in.good())
		throw bad_init;

	buf=new unsigned char[buf_size];

	refill();
}

void ibuffer::refill()
{
	if (!in.good() )
		throw bad_refill;

	in.read((char*) buf, sizeof(char)*buf_size);
	data_size = in.gcount();

	if (!in.good() && !in.eof())
		throw bad_refill;

	cur_byte=0;
}

unsigned char ibuffer::get_byte()
{
	if (cur_byte < data_size){
		unsigned char byte = buf[cur_byte];
		cur_byte++;
		return  byte;
	}
	else {
		refill();
		unsigned char byte = buf[cur_byte];
		cur_byte++;
		return  byte;
	}
}

Bit_stream::Bit_stream(const char* fname, int offset): buffer(fname, offset)
{
	cur_byte = buffer.get_byte();

	bit_pos = 0;

}


bool Bit_stream::get_bit()
{
	static unsigned char bit_masks[8] = {1, 2, 4, 8, 16, 32, 64, 128};

	int bit = cur_byte>>bit_pos;
}
