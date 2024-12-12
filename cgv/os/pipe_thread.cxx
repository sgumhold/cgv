#include "pipe.hpp"
#include "pipe_thread.h"
#include "cmdline_tools.h"

#include <cstring>
#include <mutex>

namespace cgv {
	namespace os {

queued_output_thread::queued_output_thread(bool _is_binary, unsigned _ms_to_wait)
{
	is_binary = _is_binary;
	ms_to_wait = _ms_to_wait;
}
void queued_output_thread::run()
{
	if (!connect_to_child_process())
		return;
	m.lock();
	connected = true;
	m.unlock();
	while (!have_stop_request()) {
		// check for queued block
		std::pair<char*, size_t> block = { 0,0 };
		m.lock();
		if (!blocks.empty()) {
			block = blocks.front();
			blocks.pop_front();
		}
		m.unlock();
		// if this is available, 
		if (block.first) {
			// send it to the pipe and delete block
			write_block_to_pipe(block.first, block.second);
			delete[] block.first;
		}
		else {
			// otherwise check for termination
			bool all_sent;
			m.lock();
			all_sent = all_data_sent;
			m.unlock();
			if (all_sent)
				break;
			wait(ms_to_wait);
		}
	}
	// in case of stop request or done, close pipe
	close();
}
bool queued_output_thread::has_connection() const
{
	bool result;
	m.lock();
	result = connected;
	m.unlock();
	return result;
}
bool queued_output_thread::send_block(const char* data, size_t count)
{
	bool all_sent;
	m.lock();
	all_sent = all_data_sent;
	m.unlock();
	if (all_sent)
		return false;
	bool out_of_memory = false;
	char* block;
	try {
		block = new char[count];
	}
	catch (const std::bad_alloc&) {
		out_of_memory = true;
	}
	if (out_of_memory)
		return false;
	std::copy(data, data + count, block);
	m.lock();
	blocks.push_back({ block, count });
	m.unlock();
	return true;
}
size_t queued_output_thread::get_nr_blocks() const
{
	size_t nr_blocks;
	m.lock();
	nr_blocks = blocks.size();
	m.unlock();
	return nr_blocks;
}
size_t queued_output_thread::get_nr_bytes() const
{
	size_t nr_bytes = 0;
	m.lock();
	for (auto b : blocks)
		nr_bytes += b.second;
	m.unlock();
	return nr_bytes;
}
void queued_output_thread::done()
{
	m.lock();
	all_data_sent = true;
	m.unlock();
}

named_pipe_output_thread::named_pipe_output_thread(const std::string& _pipe_name, bool _is_binary, unsigned _ms_to_wait)
	: queued_output_thread(_is_binary, _ms_to_wait)
{
	pipe_name = _pipe_name;
}
std::string named_pipe_output_thread::get_pipe_path() const
{
	return nes::pipe_root + pipe_name;
}
bool named_pipe_output_thread::connect_to_child_process()
{
	auto mode = is_binary ? (std::ios_base::out | std::ios_base::binary) : std::ios_base::out;
	pipe_ptr = new nes::pipe_ostream(pipe_name, mode);
	if (pipe_ptr->fail()) {
		delete pipe_ptr;
		pipe_ptr = 0;
		return false;
	}
	return true;
}
void named_pipe_output_thread::write_block_to_pipe(const char* data, size_t count)
{
	pipe_ptr->write(data, count);
}
void named_pipe_output_thread::close()
{
	pipe_ptr->close();
	delete pipe_ptr;
	pipe_ptr = 0;
}

pipe_output_thread::pipe_output_thread(const std::string& _cmd, bool _is_binary, unsigned _ms_to_wait)
	: queued_output_thread(_is_binary, _ms_to_wait)
{
	cmd = _cmd;
}
bool pipe_output_thread::connect_to_child_process()
{
	fp = cgv::os::open_system_input(cmd, is_binary);
	return fp != 0;
}
void pipe_output_thread::write_block_to_pipe(const char* data, size_t count)
{
	fwrite(data, 1, count, fp);
}
void pipe_output_thread::close()
{
	result = cgv::os::close_system_input(fp);
}
int pipe_output_thread::get_result() const
{
	return result;
}

queued_input_thread::queued_input_thread(bool _is_binary, size_t _package_size, size_t _packeges_per_block,
										 unsigned _ms_to_wait)
{
	is_binary = _is_binary;
	ms_to_wait = _ms_to_wait;
	package_size = _package_size;
	packages_per_block = _packeges_per_block;
	package_index = 0;

	packages = new char[package_size * packages_per_block];
}

void queued_input_thread::run() 
{
	if (!connect_to_source())
		return;

	while (!have_stop_request()) {
		// Allocate buffer for reading
		
		char* buffer = new char[package_size];
		size_t bytes_read = read_package_from_pipe(buffer,package_size);

		if (bytes_read > 0) {
			// Add block to queue if data was read
			mutex_packages.lock();
			memcpy(packages + package_size * package_index, buffer, bytes_read);
			package_index++;
			mutex_packages.unlock();
			if (package_index == packages_per_block) {
				move_packages_to_data_blocks();
			}
			delete[] buffer;
		}
		else {
			// No data, check for termination
			bool done;
			mutex_packages.lock();
			done = all_data_received;
			mutex_packages.unlock();
			delete[] buffer;
			if (done)
				break;
			wait(ms_to_wait);
		}
	}

	// Clean up and close source
	close();
}

void queued_input_thread::move_packages_to_data_blocks() {

	char* data = new char[package_size * packages_per_block];

	mutex_packages.lock();

	memcpy(data, packages, package_size * packages_per_block);
	package_index = 0;

	mutex_packages.unlock();

	mutex_data_blocks.lock();
	data_blocks.emplace_back(std::vector<char>(data, data + package_size * packages_per_block));
	mutex_data_blocks.unlock();

}

bool queued_input_thread::pop_data_block(char* buffer)
{
	mutex_data_blocks.lock();
	if (data_blocks.empty()) {
		mutex_data_blocks.unlock();
		return false;
	}

	auto block = std::move(data_blocks.front());
	data_blocks.pop_front();
	mutex_data_blocks.unlock();
	memcpy(buffer, block.data(),package_size * packages_per_block);

	return true;
}
size_t queued_input_thread::get_nr_blocks() const
{ 
	size_t nr_blocks;
	mutex_data_blocks.lock();
	nr_blocks = data_blocks.size();
	mutex_data_blocks.unlock();
	return nr_blocks;
}
void queued_input_thread::done()
{
	mutex_packages.lock();
	all_data_received = true;
	mutex_packages.unlock();
}
bool named_pipe_input_thread::connect_to_source() 
{ 
	auto mode = is_binary ? (std::ios_base::in | std::ios_base::binary) : std::ios_base::in;
	pipe_ptr = new nes::pipe_istream(pipe_name, mode);
	if (pipe_ptr->fail()) {
		delete pipe_ptr;
		pipe_ptr = 0;
		return false;
	}
	return true;
}
size_t named_pipe_input_thread::read_package_from_pipe(char* buffer,size_t package_size) {
	if (!pipe_ptr->read(buffer, package_size)) {
		mutex_packages.lock();
		all_data_received = true;
		mutex_packages.unlock();
		return 0;
	}
	return pipe_ptr->gcount();
}
void named_pipe_input_thread::close() 
{
	if (pipe_ptr) {
		pipe_ptr->close();
		delete pipe_ptr;
		pipe_ptr = 0;	
	}
}
named_pipe_input_thread::named_pipe_input_thread(const std::string& _pipe_name, bool is_binary,
						size_t _package_size, size_t _packeges_per_block, unsigned _ms_to_wait)
	: queued_input_thread(is_binary,_package_size,_packeges_per_block, _ms_to_wait)
{
	pipe_name = _pipe_name;
}
std::string named_pipe_input_thread::get_pipe_path() const 
{ 
	return nes::pipe_root + pipe_name; 
}
bool pipe_input_thread::connect_to_source() { 
	fp = cgv::os::open_system_output(cmd, is_binary);
	return fp != 0;
}
size_t pipe_input_thread::read_package_from_pipe(char* buffer, size_t package_size)
{ 
	if (!fread(buffer, 1, package_size, fp)) {
		mutex_packages.lock();
		all_data_received = true;
		mutex_packages.unlock();
		return 0;
	}
	return package_size;
}
void pipe_input_thread::close() 
{ 
	result = cgv::os::close_system_input(fp); 
}
pipe_input_thread::pipe_input_thread(const std::string& _cmd, bool is_binary, size_t _package_size,size_t _packages_per_block, unsigned _ms_to_wait)
	: queued_input_thread(is_binary, _package_size, _packages_per_block, _ms_to_wait)
{
	cmd = _cmd;
}
int pipe_input_thread::get_result() const { 
	return result; 
}
	} // namespace os
}

