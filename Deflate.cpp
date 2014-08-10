#include "Deflate.h"
#include "Huf_tree.h"

const int END_OF_BLOCK = 256 ;

void obuffer::put_byte(unsigned char byte)
{
	if (!out.is_open())
		throw ;
	if (!out.good())
		throw ;

	//// when buffer reach definite size write it into ofstream
	if ( buf.size() == OUTBUF_SZ){
		out.write((char *) &buf[0], sizeof(char) * buf.size());
		buf.clear();
	}

	////add another byte to the buffer
	buf.push_back(byte);
}

void obuffer::write(const std::vector<unsigned char>& data )
{
	//// when buffer reach definite size write it into ofstream
	if ( buf.size() == OUTBUF_SZ){
		out.write((char *) &buf[0], sizeof(char) * buf.size());
		buf.clear();
	}

	/* if data contains more bytes than the amount of available
	 * bytes in a buffer,
	 */
	if (data.size() > OUTBUF_SZ - buf.size()  ){
		std::vector<unsigned char>::const_iterator p = data.begin() +  (OUTBUF_SZ - buf.size() + 1);
		buf.insert(buf.end(), data.begin(), p);
		out.write((char *) &buf[0], sizeof(char) * buf.size());
		buf.clear();
		buf.insert(buf.end(),  p, data.end());
	}
	else
		buf.insert(buf.end(), data.begin(), data.end());

}

void obuffer::close()
{
	if (!out.is_open())
		throw ;

	out.write((char *) &buf[0], sizeof(char) * buf.size());
	buf.clear();
	out.close();
}



void wnd32k::put_byte(unsigned char byte)
{
	if (wnd.size() < WND_SZ)
		wnd.push_back(byte);
	else
	{
		wnd.push_back(byte);
		wnd.erase(wnd.begin());
	}
}

void wnd32k::append(const std::vector<unsigned char>& v )
{
	wnd.insert(wnd.end(), v.begin(), v.end()) ;

	if (wnd.size() > WND_SZ){
		int nelem = wnd.size() - WND_SZ +1;
		wnd.erase(wnd.begin(), wnd.begin() + nelem);
	}

}

std::vector<unsigned char> wnd32k::retrieve(int len, int dist)
{
	//defining index of an element to start copy bytes from
	std::vector<unsigned char>::iterator ith = wnd.begin() + wnd.size() - dist;

	std::vector<unsigned char> retrv_bytes;
	retrv_bytes.reserve(len);
	if (len > dist)
	{
		retrv_bytes.insert(retrv_bytes.end(), ith, wnd.end());

		while ( retrv_bytes.size() < (unsigned int) len ){
			ith = wnd.begin() + wnd.size() - dist;
			for ( ; ith!=wnd.end() && retrv_bytes.size() < (unsigned int) len ; ith++)
				retrv_bytes.push_back(*ith);
		}
	}
	else
		retrv_bytes.insert(retrv_bytes.end(), ith, ith + len);

	return retrv_bytes;
}


Deflate_stream::Deflate_stream(const char* fin, const char* fout, int offset) :
		btstr(fin, offset), out_buffer(fout)
{
	last_block = false;

	const int LEN_CODE_COUNT = 29;
	const int DIST_CODE_COUNT = 30;
	struct extra_bits len_bts[LEN_CODE_COUNT]={
			{0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, {0, 10},
			{1, 11}, {1, 13}, {1, 15}, {1, 17},
			{2, 19}, {2, 23}, {2, 27}, {2, 31},
			{3, 35}, {3, 43}, {3, 51}, {3, 59},
			{4, 67}, {4, 83}, {4, 99}, {4, 115},
			{5, 131}, {5, 163}, {5, 195}, {5, 227},
			{0, 258}
	};

	struct extra_bits dist_bts[DIST_CODE_COUNT]={
			{0, 1}, {0, 2},{0, 3},{0, 4},	{1, 5}, {1, 7},	{2, 9}, {2, 13},
			{3, 17}, {3, 25},		{4, 33},	{4, 49},	{5, 65}, {5, 97},
			{6, 129}, {6, 193},		{7, 257},	{7, 385},	{8, 513},{8, 769},
			{9, 1025}, {9, 1537},	{10, 2049}, {10, 3073},
			{11, 4097}, {11, 6145},	{12, 8193}, {12, 12289},
			{13, 16385}, {13, 24577}
	};


	int i=0;
	for (int code=257; code<=285; code++, i++)
		lengths[code] = len_bts[i];

	for (int code=0; code<=29; code++)
		distances[code] = dist_bts[code];

}

tree::Huf_tree Deflate_stream::init_fixtree()
{
	const int CODES_COUNT = 288;
	std::vector<tree::pair> code_lenths(CODES_COUNT);

	int litval;
	for (litval=0; litval<=143; litval++)
		code_lenths.push_back( tree::pair(litval, 8));

	for (litval=144; litval<=255; litval++)
		code_lenths.push_back(tree::pair(litval,9));

	for (litval=256; litval<=279; litval++)
		code_lenths.push_back(tree::pair(litval,7));

	for (litval=280; litval<=287; litval++)
		code_lenths.push_back(tree::pair(litval,8));

	return tree::Huf_tree(code_lenths);

}

void Deflate_stream::decode_block()
{
	last_block = btstr.get_bit();

	int block_type = btstr.read_reverse(2);

	if (block_type == 0)
		uncompressed();
	else if (block_type == 1)
		fixed_huf();
	else if (block_type == 2)
		dynamic_huf();
	else
		throw ;

}

void Deflate_stream::uncompressed()
{
	btstr.skip_bits();

	unsigned short len = btstr.get_wordLE();
	unsigned short nlen = btstr.get_wordLE();

	if ( len != (unsigned short ) ~nlen )
		throw ;

	//copy bytes from btstr to out
	for (int i=0 ; i < len; i++){
		unsigned char byte = btstr.get_byte();
		w.put_byte( byte );
		out_buffer.put_byte( byte );
	}

}

void Deflate_stream::fixed_huf()
{

	static tree::Huf_tree fix_tr = init_fixtree();

	int litlen_code;
	while ( ( litlen_code=get_value(fix_tr) ) != END_OF_BLOCK )
		if (litlen_code<0 || litlen_code>285)
			throw ;
		else if (litlen_code<256){
			w.put_byte(litlen_code);
			out_buffer.put_byte(litlen_code);
		}
		else if (litlen_code>256){
			int len = lengths[litlen_code].min_val + btstr.read_reverse(lengths[litlen_code].bits);

			int dist_code = btstr.read_reverse(5);

			int dist = distances[dist_code].min_val + btstr.read_reverse(distances[dist_code].bits);

			copy_bytes(len, dist);
		}

}


void Deflate_stream::dynamic_huf()
{

	int nlitlen	  = btstr.read_reverse(5) + 257;
	int ndist     = btstr.read_reverse(5) + 1;
	int nlengths  = btstr.read_reverse(4) + 4;

	tree::Huf_tree length_htree = get_lenghts_tree(nlengths);

	tree::Huf_tree litlen_htree = decode_rle(length_htree,nlitlen);

	tree::Huf_tree dist_htree = decode_rle(length_htree, ndist);

	int litlen_code;
	while ( ( litlen_code=get_value(litlen_htree) ) != END_OF_BLOCK )
		if (litlen_code<0 || litlen_code>285)
			throw ;
		else if (litlen_code<256){
			w.put_byte(litlen_code);
			out_buffer.put_byte(litlen_code);
		}
		else if (litlen_code>256){
			int len = lengths[litlen_code].min_val + btstr.read_reverse(lengths[litlen_code].bits);

			int dist_code = get_value(dist_htree);

			int dist = distances[dist_code].min_val + btstr.read_reverse(distances[dist_code].bits);

			copy_bytes(len, dist);
		}
}

tree::Huf_tree Deflate_stream::get_lenghts_tree(int count)
{
	const int MAX_CODE_COUNT = 19;

	static struct tree::pair x[MAX_CODE_COUNT] = {
			tree::pair(16, 0), tree::pair(17, 0), tree::pair(18, 0), tree::pair(0, 0),
			tree::pair(8,0),   tree::pair(7, 0),  tree::pair(9, 0),  tree::pair(6, 0),
			tree::pair(10, 0), tree::pair(5, 0),  tree::pair(11,0),  tree::pair(4, 0),
			tree::pair(12, 0), tree::pair(3, 0),  tree::pair(13, 0), tree::pair(2, 0),
			tree::pair(14, 0), tree::pair(1, 0),  tree::pair(15, 0)
	};
	std::vector<tree::pair> code_lengths( &x[0], &x[0]+count );

	std::vector<tree::pair>::iterator p = code_lengths.begin();
	for ( ; p != code_lengths.end(); p++ )
		p->length = btstr.read_reverse(3);

	return tree::Huf_tree(code_lengths);
}

tree::Huf_tree Deflate_stream::decode_rle(const tree::Huf_tree& htr, int count)
{
	std::vector<tree::pair> alphabt;
	alphabt.reserve(count);

	int symbol_ith=0;
	while ( symbol_ith < count  ){
		int length = get_value(htr);

		if (length < 0)
			throw ;
		else if (length < 16){
			alphabt.push_back( tree::pair(symbol_ith,length) );
			symbol_ith++;
		}
		else if (length == 16){
			int repeat_counter = btstr.read_reverse(2) + 3;

			struct tree::pair prev = alphabt[alphabt.size()-1];
			for (int n=0; n < repeat_counter && symbol_ith < count; n++,symbol_ith++)
				alphabt.push_back( tree::pair(symbol_ith, prev.length) );
		}
		else if (length == 17){
			int repeat_counter = btstr.read_reverse(3) + 3;
			for (int n=0; n < repeat_counter && symbol_ith < count; n++,symbol_ith++)
				alphabt.push_back( tree::pair(symbol_ith, 0) );
		}
		else if (length == 18){
			int repeat_counter = btstr.read_reverse(7) + 11;
			for (int n=0; n < repeat_counter && symbol_ith < count; n++,symbol_ith++)
				alphabt.push_back( tree::pair(symbol_ith, 0) );
		}
		else
			throw ;
	}

	return tree::Huf_tree(alphabt);
}


void Deflate_stream::copy_bytes(int len, int dist)
{
	std::vector<unsigned char> v;

	v = w.retrieve(len, dist);

	w.append(v);

	out_buffer.write(v);
}

int Deflate_stream::get_value(const tree::Huf_tree& htr)
{
	int x;
	do{
		if (btstr.get_bit()==0)
			x=htr.down_left();
		else
			x=htr.down_right();
	}
	while ( x==tree::NOVALUE );

	return x;
}

