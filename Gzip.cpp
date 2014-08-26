#include "Gzip.h"
#include "Deflate.h"
#include "boost/crc.hpp"

namespace gzip {

Gzip_stream::Gzip_stream(boost::filesystem::path&  fin): in()
{
	const std::streamoff CRC_OFFSET = 8; //offset from the end of the file

	in.open(fin, std::ios::in | std::ios::binary);
	if (!in.is_open())
		throw bad_fopen();

	archive_name = fin;
	extra_f = NULL;
	crc16 = 0;

	read_flds();

	if ( fields.flg & FTEXT )
		is_txt = true;
	else
		is_txt = false;

	if ( fields.flg & FEXTRA )
		read_fextra();

	if ( fields.flg & FNAME)
		read_fname();

	if ( fields.flg & FCOMMENT )
		read_fcomment();

	if ( fields.flg & FHCRC )
		in.read((char*) & crc16, sizeof(unsigned short));

	offset = in.tellg();

	in.seekg(-CRC_OFFSET, std::ios::end );
	in.read((char *) &(fields.crc32), sizeof(unsigned long));
	in.read((char *) &(fields.isize), sizeof(unsigned long));
	in.close();
}

void Gzip_stream::decode()
{
	boost::filesystem::path output_fn = output_fname();

	size_t input_sz = boost::filesystem::file_size(archive_name);
	deflate::Deflate_stream str(archive_name, output_fn, offset, input_sz);

	do {
		str.decode_block();
	}
	while(!str.last_blck());

	str.close();

}


boost::filesystem::path  Gzip_stream::output_fname()
{
	////extract path to the archive file
	boost::filesystem::path output_fn = archive_name.parent_path();


	////add the name of an archive itself (not including .gz)
	if (fname !=""){
		boost::filesystem::path p(fname);
		output_fn /= p;
	}
	else if ( archive_name.stem().string() =="")
		output_fn /= "unpacked_with_Gzip_decoder";
	else
		output_fn /= archive_name.stem() ;

	return output_fn ;
}



void Gzip_stream::read_flds()
{
	in.read((char*) &(fields.id1), sizeof(char));
	if (fields.id1 != ID1)
		throw bad_id();

	in.read((char*) &(fields.id2), sizeof(char));
	if (fields.id2 != ID2)
		throw bad_id();

	in.read((char*) &(fields.cm), sizeof(char));
	if (fields.cm != CM)
		throw bad_format();

	in.read((char*) &(fields.flg), sizeof(char));

	in.read((char*) &(fields.mtime), sizeof(unsigned int));

	in.read((char*) &(fields.xfl), sizeof(char));

	in.read((char*) &(fields.os), sizeof(char));
}

void Gzip_stream::read_fextra()
{
	unsigned short xlen ;

	in.read((char *) &xlen, sizeof(unsigned short));

	unsigned char si1, si2;

	in.read((char*) &si1, sizeof(char));
	in.read((char*) &si2, sizeof(char));

	unsigned short data_len;
	in.read((char*) &data_len, sizeof(unsigned short));

	if (xlen != data_len + 4)
		throw bad_format();

	std::vector<unsigned char> data(data_len);

	in.read((char*) &data[0], sizeof(char) * data_len);

	extra_f = new Extra_field(si1, si2, data);
}

void Gzip_stream::read_fname()
{
	char ch;
	while ( (ch = in.get())!='\0')
		fname.push_back(ch);

	fname.push_back('\0');
}

void Gzip_stream::read_fcomment()
{
	char ch;
	while ( (ch = in.get())!='\0')
		fcomment.push_back(ch);

	fcomment.push_back('\0');
}

bool Gzip_stream::get_xfield(Extra_field& xfield)
{
	if (extra_f){
		xfield = *extra_f;
		return true;
	}
	else
		return false;
}

bool Gzip_stream::is_correct()
{
	boost::crc_32_type  crc32;
	boost::filesystem::path  full_fn = output_fname();
	boost::filesystem::ifstream  ifs( full_fn, std::ios_base::binary );

	std::vector<char>  buffer(BUFFER_SIZE * 1024);

	do
	{
		ifs.read( &buffer[0], buffer.size() );
		crc32.process_bytes( &buffer[0], ifs.gcount() );
	} while ( ifs );

	ifs.close();
	return crc32.checksum() == fields.crc32 ;

}

}

