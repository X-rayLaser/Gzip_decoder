#include "Bit_stream.h"

namespace btstream
{

ibuffer::ibuffer(boost::filesystem::path& fname, int offset ): in(), buf(INBUF_SZ * 1024)
{
	in.open(fname, std::ios::in | std::ios::binary);
	if (!in.is_open())
		throw bad_open();

	in.seekg(offset);

	if (!in.good())
		throw bad_init();

	is_end = false;
	refill();
}

void ibuffer::refill()
{
	if (!in.good() )
		throw bad_refill();

	in.read((char*) &buf[0], sizeof(char)*buf.size());

	buf_end = buf.begin() + in.gcount();
	if (buf_end == buf.begin() )		 //if the end of file was achieved
		is_end = true;

	if (!in.good() && !in.eof())
		throw bad_refill();

	pbuf=buf.begin();
}


int ibuffer::read(std::vector<unsigned char>& dest, int count)
{
	if (count == 0)
		return 0;

	int buffer_space = buf_end - pbuf ;
	if (count > buffer_space){
		int remain_count = count - buffer_space;
		if (pbuf != buf_end)
			dest.insert(dest.end(), pbuf, buf_end);

		refill();
		if (is_end)
			return buffer_space ;

		if (pbuf + remain_count <= buf_end){
			dest.insert(dest.end(), buf.begin(), buf.begin() + remain_count );
			pbuf += remain_count;  //new bytepos
			return buffer_space + remain_count ;
		}

		dest.insert(dest.end(), pbuf , buf_end );
		pbuf = buf_end;
		return buffer_space + buf_end - buf.begin()  ;
	}
	else{
		dest.insert(dest.end(), pbuf,  pbuf + count);
		pbuf += count ;  //new bytepos

		return count ;
	}
}


Bit_stream::Bit_stream(boost::filesystem::path& fname, int offset ): in_buffer(fname, offset)
{
	cur_byte = in_buffer.get_byte();

	bit_pos = 0;

}


bool Bit_stream::get_bit()
{
	if (bit_pos == 8){
		cur_byte = in_buffer.get_byte();
		bit_pos = 0;
	}


	if (bit_pos == 0){
		bit_pos++;
		return (cur_byte & 1) == 1;
	}

	return ( (cur_byte >> bit_pos++) & 1) == 1;
}

/* read a given number of bits in MSB order */
int Bit_stream::read_reverse(int bit_count)
{
	if (bit_count == 0)
		return 0;

	int res = 0;

	res = res | get_bit();
	for (int i = 1; i < bit_count; i++)
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



}
