#include "Deflate.h"
#include "Huf_tree.h"
Dft_decoder::Dft_decoder(const char *in)
{
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

	struct extra_bitx dist_bts[DIST_CODE_COUNT]={
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

tree::Huf_tree Dft_decoder::init_fixtree()
{
	const int CODES_COUNT = 287;
	std::vector<tree::pair> code_lenths(CODES_COUNT);

	int litval;
	for (litval=0; litval<=143; litval++)
		code_lenths.push_back( {litval, 8});

	for (litval=144; litval<=255; litval++)
		code_lenths.push_back({litval,9});

	for (litval=256; litval<=279; litval++)
		code_lenths.push_back({litval,7});

	for (litval=280; litval<=287; litval++)
		code_lenths.push_back({litval,8});

	return tree::Huf_tree(code_lenths);

}

void Dft_decoder::decode_block()
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

void Dft_decoder::uncompressed()
{
	btstr.skip_bits();

	unsigned short len = btstr.get_wordLE();
	unsigned short nlen = btstr.get_wordLE();

	if ( len != ~nlen )
		throw ;

	//copy bytes from btstr to out
	for (int i=0 ; i < len; i++)
		out.push_back( btstr.get_byte() );

}

void Dft_decoder::fixed_huf()
{

	static tree::Huf_tree fix_tr = init_fixtree();

	int litlen_code;
	while ( ( litlen_code=get_value(fix_tr) ) != 256 )
		if (litlen_code<0 || litlen_code>285)
			throw ;
		else if (litlen_code<256)
			out.push_back(litlen_code);
		else if (litlen_code>256){
			int len = lengths[litlen_code].min_val + btstr.read_reverse(lengths[litlen_code].bits);

			int dist_code = btstr.read_reverse(5);

			int dist = distances[dist_code].min_val + btstr.read_reverse(distances[dist_code].bits);

			copy_bytes(len, dist);
		}

}


void Dft_decoder::dynamic_huf()
{

	int nlitlen	  = btstr.read_reverse(5) + 257;
	int ndist     = btstr.read_reverse(5) + 1;
	int nlengths  = btstr.read_reverse(4) + 4;

	tree::Huf_tree length_htree = get_lenghts_tree(nlengths);

	tree::Huf_tree litlen_htree = decode_rle(length_htree,nlitlen);

	tree::Huf_tree dist_htree = decode_rle(length_htree, ndist);

	int litlen_code;
	while ( ( litlen_code=get_value(litlen_htree) ) != 256 )
		if (litlen_code<0 || litlen_code>285)
			throw ;
		else if (litlen_code<256)
			out.push_back(litlen_code);
		else if (litlen_code>256){
			int len = lengths[litlen_code].min_val + btstr.read_reverse(lengths[litlen_code].bits);

			int dist_code = get_value(dist_htree);

			int dist = distances[dist_code].min_val + btstr.read_reverse(distances[dist_code].bits);

			copy_bytes(len, dist);
		}
}

tree::Huf_tree Dft_decoder::get_lenghts_tree(int count)
{
	const int MAX_CODE_COUNT = 19;

	static struct tree::pair x[MAX_CODE_COUNT] = {
			{16, 0}, {17, 0}, {18, 0}, {0, 0}, {8,0}, {7, 0},
			{9, 0}, {6, 0}, {10, 0}, {5, 0}, {11,0}, {4, 0}, {12, 0},
			{3, 0}, {13, 0}, {2, 0}, {14, 0}, {1, 0}, {15, 0}
	};
	std::vector<tree::pair> code_lengths( &x[0], &x[0]+count );

	std::vector<tree::pair>::iterator p = code_lengths.begin();
	for ( ; p != code_lengths.end(); p++ )
		p->length = btstr.read_reverse(3);

	return tree::Huf_tree(code_lengths);
}

tree::Huf_tree Dft_decoder::decode_rle(const tree::Huf_tree& htr, int count)
{
	std::vector<tree::pair> alphabt;
	alphabt.reserve(count);

	int symbol_ith=0;
	while ( symbol_ith < count  ){
		int length = get_value(htr);

		if (length < 0)
			throw ;
		else if (length < 16){
			alphabt.push_back( {symbol_ith,length} );
			symbol_ith++;
		}
		else if (length == 16){
			int repeat_counter = btstr.read_reverse(2) + 3;

			struct tree::pair prev = alphabt[alphabt.size()-1];
			for (int n=0; n < repeat_counter && symbol_ith < count; n++,symbol_ith++)
				alphabt.push_back( {symbol_ith, prev.length} );
		}
		else if (length == 17){
			int repeat_counter = btstr.read_reverse(3) + 3;
			for (int n=0; n < repeat_counter && symbol_ith < count; n++,symbol_ith++)
				alphabt.push_back( {symbol_ith, 0 } );
		}
		else if (length == 18){
			int repeat_counter = btstr.read_reverse(7) + 11;
			for (int n=0; n < repeat_counter && symbol_ith < count; n++,symbol_ith++)
				alphabt.push_back( {symbol_ith, 0 } );
		}
		else
			throw ;
	}

	return tree::Huf_tree(alphabt);
}


void Dft_decoder::copy_bytes(int len, int dist)
{
	vector<unsigned char>::iterator ith = out.begin() + out.size() - dist;
	if (len > dist){
		vector<unsigned char> buf;
		buf.reserve(dist);
		buf.insert(buf.end(), ith, out.end());
		out.insert(out.end(), buf.begin(), buf.end());

		int i=0;
		while (i<len){
			ith = buf.begin();
			while (ith!=buf.end() && i<len){
				out.push_back(*ith);
				ith++;
				i++;
			}
		}
	}
	else
	{
		vector<unsigned char> buf;
		buf.reserve(len);
		buf.insert(buf.end(), ith, ith+len);
		out.insert(out.end(), buf.begin(), buf.end());
	}

}

int Dft_decoder::get_value(const tree::Huf_tree& htr)
{
	int x;
	do{
		if (btstr.get_bit()==0)
			x=htr.down_left();
		else
			x=htr.down_right();
	}
	while ( x==tree::Huf_tree::NOVALUE );

	return x;
}
