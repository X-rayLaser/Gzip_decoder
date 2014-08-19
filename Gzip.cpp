#include "Gzip.h"
#include "Deflate.h"

namespace gzip {

const unsigned char ID1 = 31;
const unsigned char ID2 = 139;
const unsigned char CM  = 8;



Gzip_stream::Gzip_stream(const std::string fin): in()
{
	const int CRC_OFFSET = 8; //offset from the end of the file

	in.open(fin.c_str(), std::ios::in | std::ios::binary);
	if (!in.is_open())
		throw bad_fstate();

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

	in.seekg(CRC_OFFSET, std::ios::end );
	in.read((char *) &fields.crc32, sizeof(unsigned long));
	in.read((char *) & fields.isize, sizeof(unsigned long));
	in.close();
}

void Gzip_stream::decode()
{
	std::string output_fn = output_fname();

	Deflate_stream str(archive_name.c_str(), output_fn.c_str(), offset);

	do {
		str.decode_block();
	}
	while(!str.last_blck());

	str.close();

}

std::string Gzip_stream::output_fname()
{
	////extract path to the archive file and extract the name
	////of an archive itself (not including .gz)
	unsigned int indx = archive_name.find_last_of("/\\");
	std::string arch_dir = archive_name.substr(0,indx);
	std::string archive_file = archive_name.substr(indx  + 1);

	if (fname !="")
		return arch_dir + "\\" + fname;

	////if a field fname is absent in the gzip header then:
	unsigned int ch_indx = archive_file.find_last_not_of('.');
	std::string s = archive_file.substr(0,ch_indx);
	if (s=="")
		return arch_dir + "\\" + "unpacked_with_Gzip_decoder";
	else
		return arch_dir + "\\" + s;
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


}

