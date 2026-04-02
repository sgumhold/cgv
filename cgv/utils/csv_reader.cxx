#include "csv_reader.h" 
#include <cgv/utils/file.h>

namespace cgv {
	namespace utils {

const char* csv_reader_base::get_ws() const 
{
	return ((flags & CSV_SPACE) != 0) ? " " : ""; 
}
const char* csv_reader_base::get_sep() const 
{
	if ((flags & CSV_COMMA) != 0) {
		if ((flags & CSV_TAB) != 0)
			return ",\t";
		else
			return ",";
	}
	else
		if ((flags & CSV_TAB) != 0)
			return "\t";
	return "";
}
bool csv_reader_base::has_heading() const 
{
	return (flags & CSV_HEADING) != 0; 
}
bool csv_reader_base::fail() const 
{
	return failed;
}
bool csv_reader_base::find_column_index(const std::string& col_name, size_t& col_index) const
{
	auto iter = std::find(col_names.begin(), col_names.end(), col_name);
	if (iter == col_names.end())
		return false;
	col_index = unsigned(iter - col_names.begin());
	return true;
}
csv_reader_base::csv_reader_base(const std::string& file_name, CSV_Flags _flags)
{
	flags = _flags;
	if (!cgv::utils::file::read(file_name, content, true)) {
		last_error = "cannot read <" + file_name + ">";
		failed = true;
		return;
	}
	cgv::utils::split_to_lines(content, lines, true);
	while (li < lines.size()) {
		if (!lines[li].empty())
			break;
		++li;
	}
	if (li >= lines.size()) {
		last_error = "all lines in csv file are empty";
		failed = true;
		return;
	}
	if (has_heading()) {
		std::vector<cgv::utils::token> tokens;
		cgv::utils::split_to_tokens(lines[li], tokens, "", false, "", "", get_sep());
		for (auto tok : tokens)
			col_names.push_back(cgv::utils::to_string(tok));
		++li;
		nr_cols = col_names.size();
	}
}

bool csv_reader_base::parse_next_line(std::vector<cgv::utils::token>& tokens) const
{
	while (li < lines.size()) {
		if (lines[li].empty()) {
			++li;
			continue;
		}
		// split with seperator extration to support empty entries 
		std::vector<cgv::utils::token> sep_tokens;
		cgv::utils::split_to_tokens(lines[li], sep_tokens, get_sep(), false, "", "", get_ws());
		// filter out separators
		tokens.clear();
		bool last_was_empty = true;
		for (auto tok : sep_tokens) {
			if (tok == "," || tok == "\t") {
				if (last_was_empty)
					tokens.push_back(cgv::utils::token(tok.begin, tok.begin));
				else
					last_was_empty = true;
			}
			else {
				tokens.push_back(tok);
				last_was_empty = false;
			}
		}
		if (nr_cols == 0)
			nr_cols = tokens.size();
		else
			if (tokens.size() < col_names.size()) {
				++li;
				continue;
			}
		++li;
		return true;
	}
	return false;
}

	}
}