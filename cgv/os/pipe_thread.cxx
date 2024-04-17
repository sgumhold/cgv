#include "pipe.hpp"
#include "pipe_thread.h"

namespace cgv {
	namespace os {

pipe_output_thread::pipe_output_thread(const std::string& _pipe_name, bool _is_binary, unsigned _ms_to_wait)
{
	pipe_name = _pipe_name;
	is_binary = _is_binary;
	ms_to_wait = _ms_to_wait;
}
std::string pipe_output_thread::get_pipe_path() const
{
	return nes::pipe_root + pipe_name;
}
void pipe_output_thread::run()
{
	auto mode = is_binary ? (std::ios_base::out | std::ios_base::binary) : std::ios_base::out;
	pipe_ptr = new nes::pipe_ostream(pipe_name, mode);
	if (pipe_ptr->fail()) {
		delete pipe_ptr;
		pipe_ptr = 0;
		return;
	}
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
			pipe_ptr->write(block.first, block.second);
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
			wait(20);
		}
	}
	// in case of stop request or done, close pipe
	pipe_ptr->close();
	delete pipe_ptr;
	pipe_ptr = 0;
}
bool pipe_output_thread::has_connection() const
{
	bool result;
	m.lock();
	result = pipe_ptr != 0 && connected;
	m.unlock();
	return result;
}
bool pipe_output_thread::send_block(const char* data, size_t count)
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
size_t pipe_output_thread::get_nr_blocks() const
{
	size_t nr_blocks;
	m.lock();
	nr_blocks = blocks.size();
	m.unlock();
	return nr_blocks;
}
size_t pipe_output_thread::get_nr_bytes() const
{
	size_t nr_bytes = 0;
	m.lock();
	for (auto b : blocks)
		nr_bytes += b.second;
	m.unlock();
	return nr_bytes;
}
void pipe_output_thread::done()
{
	m.lock();
	all_data_sent = true;
	m.unlock();
}

	}
}

