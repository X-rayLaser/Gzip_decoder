#include "Deflate.h"
#include "Huf_tree.h"
#define WIN32_WINNt 0x0500
#define WINVER 0x0500
#include <windows.h>


const int END_OF_BLOCK = 256 ;

obuffer::obuffer(boost::filesystem::path fname, size_t finp_size) : out()
{
	size_t lim = memlimit() / 2; //to leave a system some memory

	if (lim < MIN_BUFFER_SIZE * 1024)
		throw ; //not enough memory for decompression

	if (finp_size * CMP_RATIO  > lim)
		max_size = lim;
	else {
		if (finp_size * CMP_RATIO < MIN_BUFFER_SIZE * 1024)
			max_size = MIN_BUFFER_SIZE * 1024;
		else
			max_size = finp_size * CMP_RATIO ;
	}

	data_size = 0;
	out.open(fname, std::ios::out | std::ios::binary);

	buf.resize(max_size) ;
}

size_t obuffer::memlimit()
{
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);

	return status.ullTotalPhys ;
}

void obuffer::flush_buf()
{
	int byte_count = max_size - WND_SZ;
	out.write((char *) &buf[0], sizeof(char) * byte_count);
	if (!out.good())
		throw bad_fstate();
	std::copy( buf.end() - WND_SZ, buf.end(), buf.begin() );
	data_size = WND_SZ ;
}

void obuffer::put_byte(const unsigned char& byte)
{
	if (!out.is_open())
		throw bad_fstate();
	if (!out.good())
		throw bad_fstate();

	//// when buffer reach definite size
	if ( data_size == max_size)
		flush_buf();

	////add another byte to the buffer
	buf[data_size++] = byte;
}

void obuffer::write_raw(const unsigned char* data, int size)
{
	//// when buffer reach definite size write it into ofstream
	if ( data_size == max_size)
		flush_buf();

	/* if data contains more bytes than the amount of available
	 * bytes in a buffer,
	 */
	unsigned buf_space = max_size - data_size ;
	if ( (unsigned) size > buf_space  ){
		std::copy(&data[0],  &data[0] + buf_space, &buf[data_size]);

		flush_buf();

		std::copy(&data[0] + buf_space, &data[0] + size, &buf[data_size] );
		data_size += (size - buf_space);
	}
	else{
		std::copy(&data[0], &data[0] + size, &buf[data_size] );
		data_size +=size ;
	}
}


void obuffer::copy(int len, int dist)
{
	//defining index of an element to start copy bytes from
	int first_ind = data_size - dist;

	if (len <=dist){

		write_raw( &buf[first_ind], len );
		return ;
	}

	/* retrieves address to the beginning of bytes in the vector to be copied
	 * and appends them to the same vector (buf). Copy bytes as many as dist
	 */
	while (len >= dist){
		write_raw(&buf[first_ind], dist);
		len -= dist;
	}

	/* copies remaining bytes one by one */
	int indx = first_ind;
	for (  ; len !=0 ; indx++, len--)
		put_byte( int(buf[indx]) );
}

void obuffer::close()
{
	if (!out.is_open())
		throw bad_fstate();

	out.write((char *) &buf[0], sizeof(char) * data_size);
	if (!out.good())
		throw bad_fstate();
	buf.clear();
	out.close();
}



Deflate_stream::Deflate_stream(boost::filesystem::path fin,
		boost::filesystem::path fout, int offset, size_t input_sz) :
		btstr(fin, offset), out_buffer(fout, input_sz)
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
		lengths.insert(std::make_pair(code, len_bts[i]));

	for (int code=0; code<=29; code++)
		distances.insert(std::make_pair(code,  dist_bts[code]));

}

/*construct fixed literal/length tree */
tree::Huf_tree Deflate_stream::init_lentree()
{
	const int CODES_COUNT = 288;
	std::vector<tree::pair> code_lengths;
	code_lengths.reserve(CODES_COUNT);

	int litval;
	for (litval=0; litval<=143; litval++)
		code_lengths.push_back( tree::pair(litval, 8));

	for (litval=144; litval<=255; litval++)
		code_lengths.push_back(tree::pair(litval,9));

	for (litval=256; litval<=279; litval++)
		code_lengths.push_back(tree::pair(litval,7));

	for (litval=280; litval<=287; litval++)
		code_lengths.push_back(tree::pair(litval,8));

	return tree::Huf_tree(code_lengths);

}

/*construct incomplete fixed distances tree */
tree::Huf_tree Deflate_stream::init_disttree()
{
	//maximum number of distance codes
	const int DIST_CODE_COUNT = 30;

	std::vector<tree::pair> code_lengths;
	code_lengths.reserve(DIST_CODE_COUNT);

	for (int i=0; i<DIST_CODE_COUNT; i++)
		code_lengths.push_back( tree::pair(i,5));

	return tree::Huf_tree(code_lengths);
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
		throw bad_blctype();

}

void Deflate_stream::uncompressed()
{
	btstr.skip_bits();

	unsigned short len = btstr.get_wordLE();
	unsigned short nlen = btstr.get_wordLE();

	if ( len != (unsigned short ) ~nlen )
		throw bad_chksum();

	//copy bytes from btstr to out
	std::vector<unsigned char> stored;
	stored.reserve(len);


	int count = btstr.copy(stored, len);

	if (count !=len)
		throw ;

	if (len > 0)
		out_buffer.write(stored);

}

void Deflate_stream::fixed_huf()
{

	static tree::Huf_tree len_tree  = init_lentree();
	static tree::Huf_tree dist_tree = init_disttree();

	int litlen_code;
	while ( ( litlen_code=get_value(len_tree) ) != END_OF_BLOCK )
		if (litlen_code<0 || litlen_code>285)
			throw bad_code();
		else if (litlen_code<256)
			out_buffer.put_byte(litlen_code);
		else if (litlen_code>256){
			int len = lengths[litlen_code].min_val + btstr.read_reverse(lengths[litlen_code].bits);

			int dist_code = get_value(dist_tree);
			if (dist_code >=30)
				throw bad_code();
			int dist = distances[dist_code].min_val + btstr.read_reverse(distances[dist_code].bits);

			out_buffer.copy(len, dist);
		}

}


void Deflate_stream::dynamic_huf()
{

	int nlitlen	  = btstr.read_reverse(5) + 257;
	int ndist     = btstr.read_reverse(5) + 1;
	int nlengths  = btstr.read_reverse(4) + 4;


	tree::Huf_tree length_htree = get_lenghts_tree(nlengths);

	std::vector<tree::pair> all_lenghts = decode_rle(length_htree, nlitlen, ndist);

	std::vector<tree::pair> litlen_lenths(all_lenghts.begin(),
			all_lenghts.begin() + nlitlen);

	std::vector<tree::pair> dist_lengths(all_lenghts.begin()+nlitlen,
			all_lenghts.begin() + nlitlen + ndist);

	tree::Huf_tree litlen_htree(litlen_lenths);
	tree::Huf_tree dist_htree(dist_lengths);

	int litlen_code;
	while ( ( litlen_code=get_value(litlen_htree) ) != END_OF_BLOCK )
		if (litlen_code<0 || litlen_code>285)
			throw bad_code();
		else if (litlen_code<256)
			out_buffer.put_byte(litlen_code);
		else if (litlen_code>256){
			int len = lengths[litlen_code].min_val + btstr.read_reverse(lengths[litlen_code].bits);

			int dist_code = get_value(dist_htree);
			if (dist_code >=30)
				throw bad_code();
			int dist = distances[dist_code].min_val + btstr.read_reverse(distances[dist_code].bits);

			out_buffer.copy(len, dist);
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

std::vector<tree::pair> Deflate_stream::decode_rle(const tree::Huf_tree& htr, int nlit, int ndist)
{
	std::vector<tree::pair> alphabt(nlit + ndist);
	int i ;
	for ( i=0; i < nlit; i++)
		alphabt[i].value = i;

	for (int j=0; j < ndist ; i++, j++)
		alphabt[i].value = j;

	int symbol_ith=0;
	while ( symbol_ith < nlit + ndist  ){
		int length = get_value(htr);

		if (length < 0)
			throw bad_clen();
		else if (length < 16)
			alphabt[symbol_ith++].length = length;
		else if (length == 16){
			if (symbol_ith == 0)
				throw ;
			int repeat_counter = btstr.read_reverse(2) + 3;
			int prevlength = alphabt[symbol_ith-1].length ;
			for ( ; repeat_counter-- ; symbol_ith++)
				alphabt[symbol_ith].length = prevlength;
		}
		else if (length == 17){
			int repeat_counter = btstr.read_reverse(3) + 3;
			for ( ; repeat_counter-- ; symbol_ith++)
				alphabt[symbol_ith].length = 0;
		}
		else if (length == 18){
			int repeat_counter = btstr.read_reverse(7) + 11;
			for ( ;  repeat_counter--  ;  symbol_ith++)
				alphabt[symbol_ith].length = 0;
		}
		else
			throw bad_clen();

		if (symbol_ith > nlit + ndist)
			throw ; //more codes then specified
	}

	if (alphabt[END_OF_BLOCK].length == 0)
		throw bad_codetbl();

	return alphabt ;
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

