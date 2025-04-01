#include <stdio.h>
#include <string>
#include <iostream>
#include <cgv/os/clipboard.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/type/standard_types.h>
#include <cgv/utils/advanced_scan.h>
#include <random>

// to avoid delay on shortcut press to tool launch
// you need to disable SysMain process in Services / DesktopApp which needs re-boot

//const char* ascii_chars = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
int primes[] = { 50321,	50329,	50333,	50341,	50359,	50363,	50377,	50383,	50387,	50411, 50417,	50423,	50441,	50459,	50461,	50497,	50503,	50513,	50527 };
const std::string& get_valid_chars()
{
	static std::string valid_chars = "!#%&()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_abcdefghijklmnopqrstuvwxyz{|}~";
	return valid_chars;
}
short get_valid_index(cgv::type::uint8_type c)
{
	static std::vector<short> valid_indices;
	if (valid_indices.empty()) {
		int valid_idx = 0;
		for (int h = 33; h < 128; ++h) {
			if (h == int((const cgv::type::uint8_type&)get_valid_chars()[valid_idx])) {
				valid_indices.push_back(valid_idx);
				if (++valid_idx >= get_valid_chars().size())
					break;
			}
			else
				valid_indices.push_back(-1);
		}
	}
	if (c < 33)
		return -1;
	if (c >= valid_indices.size()+33)
		return -1;
	return valid_indices[int(c) - 33];
}
std::string encode_string(const std::string& s)
{
	std::default_random_engine E(primes[13]);
	std::uniform_int_distribution<int> U(0, primes[7]);
	int n = int(get_valid_chars().size());
	std::string e;
	for (int i = 0; i < s.length(); ++i) {
		short vi = get_valid_index(s[i]);
		if (vi == -1)
			std::cerr << "ERROR: character '" << s[i] << "'(" << int(s[i]) << ") in to be encoded string invalid." << std::endl;
		else
			e.append(1, get_valid_chars()[(U(E) + vi) % n]);
	}
	return e;
}
std::string decode_string(const std::string& s)
{
	std::default_random_engine E(primes[13]);
	std::uniform_int_distribution<int> U(0, primes[7]);
	int n = int(get_valid_chars().size());
	std::string d;
	for (int i = 0; i < s.length(); ++i) {
		short vi = get_valid_index(s[i]);
		if (vi == -1)
			std::cerr << "ERROR: ignoring character " << int(s[i]) << " in to be decoded string" << std::endl;
		else {
			int u = U(E);
			d.append(1, get_valid_chars()[(vi + n * (u / n + 1) - u) % n]);
		}
	}
	return d;
}
std::string& ref_index_secret()
{
	static std::string index_secret;
	return index_secret;
}
std::string lookup_secret(int idx)
{
	if (idx < 1 || idx > 25)
		return "";
	return std::string(1, ref_index_secret()[idx - 1]);
}
std::string lookup_secrets(const std::pair<int, int>& indices)
{
	return lookup_secret(indices.first) + lookup_secret(indices.second);
}
bool parse_indices(const std::string& text, std::pair<int, int>& p)
{
	std::vector<cgv::utils::token> toks;
	cgv::utils::split_to_tokens(text, toks, "&");
	int i1 = 1, i2 = 2;
	if (toks.size() < 1)
		return false;
	if (!cgv::utils::is_integer(toks.back().begin, toks.back().end, i2))
		return false;
	if (toks.size() < 3)
		return false;
	if (!cgv::utils::is_integer(toks[toks.size() - 3].begin, toks[toks.size() - 3].end, i1))
		return false;
	p = { i1,i2 };
	return true;
}
void validate_encoding_decoding()
{
	std::default_random_engine E;
	std::uniform_int_distribution<int> U(0, int(get_valid_chars().size()) - 1);
	for (int i = 0; i < 1000000; ++i) {
		std::string s;
		for (int j = 0; j < 25; ++j) {
			s.append(1, get_valid_chars()[U(E)]);
		}
		if (encode_string(decode_string(s)) != s) {
			std::cout << "ERROR: <" << s << "> != <" << encode_string(decode_string(s)) << ">" << std::endl;
		}
		if (decode_string(encode_string(s)) != s) {
			std::cout << "ERROR: <" << s << "> != <" << decode_string(encode_string(s)) << ">" << std::endl;
		}
	}
	std::cout << "validation done." << std::endl;
}
int main(int, char**)
{
	// assume failure
	int result = 0;

	// check for index secret in environment variables
	const char* encoded_index_secret_c_str = getenv("ENCODED_INDEX_SECRET");
	if (encoded_index_secret_c_str)
		ref_index_secret() = decode_string(encoded_index_secret_c_str);
	else {
		const char* index_secret_c_str = getenv("INDEX_SECRET");
		if (index_secret_c_str)
			ref_index_secret() = index_secret_c_str;
	}
	// check for clipboard text
	std::string text;
	bool has_clipboard_text = cgv::os::get_text_from_clipboard(text);
	if (has_clipboard_text) {
		std::pair<int, int> p;
		// first try lookup mode
		if (ref_index_secret().length() == 25 && parse_indices(text, p)) {
			std::string secret = lookup_secrets(p);
			if (secret.length() == 2) {
				cgv::os::copy_text_to_clipboard(secret.c_str());
				result = 1;
			}
		}
		// otherwise try encoding mode
		else if (text.length() == 25) {
			std::string e = encode_string(text);
			if (e.length() == 25) {
				cgv::os::copy_text_to_clipboard(e.c_str());
				result = 2;
			}
		}
	}
	// if neither lookup nor encoding mode, show help
	if (result == 0) {
		std::cout
			<< "index_secret ... (does not take any arguments)\n"
			<< "    Input, mode and output via clipboard text.\n"
			<< " 1) Clipboard holds index secret query ending on '... 10 & 21'\n"
			<< "    1st-ly environment variable ENCODED_INDEX_SECRET is checked for\n"
			<< "    25 character encoded index secret, decoded and used for lookup.\n"
			<< "    2nd-ly environment variable INDEX_SECRET is checked for\n"
			<< "    25 character clear text index secret, and used for lookup.\n"
			<< "    On success, looked up 2 char secret is placed on clipboard.\n"
			<< " 2) Clipboard holds clear text 25 character index secret\n"
			<< "    secret is encoded and result placed on clipboard\n"
			<< "    use this mode for setting environment variable ENCODED_INDEX_SECRET.\n"
			<< " 0) On empty clipboard or error, this help message is shown." << std::endl;

		std::cin.get();
	}
	return result;
}