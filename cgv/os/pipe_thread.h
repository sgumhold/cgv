#pragma once

#include <deque>
#include <mutex>
#include "thread.h"
#include "lib_begin.h"

namespace nes {
	// forward declaration of pipe output stream to avoid inclusion of pipe header here
	template<typename CharT, typename Traits> class basic_pipe_ostream;
	template <typename CharT, typename Traits> class basic_pipe_istream;
}

namespace cgv {
	namespace os {
		/// <summary>
		/// base class for system command input pipe or named pipe threads including a queue of data blocks and a separate thread
		/// </summary>
		class CGV_API queued_output_thread : public cgv::os::thread
		{
			/// flag storing whether all data has been sent
			bool all_data_sent = false;
		protected:
			/// whether binary mode should be used
			bool is_binary;
			/// flag that tells whether the pipe has been connected to from the other side
			bool connected = false;
			/// mutex used to protect access to \c blocks
			mutable cgv::os::mutex m;
			/// deque used to queue the data blocks that should be written to the pipe by the thread
			std::deque<std::pair<char*, size_t> > blocks;
			/// time in miliseconds to wait while queue is empty
			unsigned ms_to_wait;
			/// to be implemented in derived classes
			virtual bool connect_to_child_process() = 0;
			/// to be implemented in derived classes
			virtual void write_block_to_pipe(const char* data, size_t count) = 0;
			/// to be implemented in derived classes
			virtual void close() = 0;
		public:
			/// construct queued output thread from flag, whether to use binary mode and wait time in ms used when queue is empty
			queued_output_thread(bool is_binary = true, unsigned _ms_to_wait = 20);
			/// connect to child process and continuously write queue content to pipe; if empty wait in intervals of \c ms_to_wait miliseconds or close pipe and terminate if done() had been called
			void run();
			/// returns true as soon as child process has connected to pipe
			bool has_connection() const;
			/// if done() had not been called, insert a data block into the queue; can fail if done() or out of memory
			bool send_block(const char* data, size_t count);
			/// returns the number of blocks in the queue of not yet written data
			size_t get_nr_blocks() const;
			/// returns the number of bytes in the queue of not yet written data, what is more time consuming than get_nr_blocks()
			size_t get_nr_bytes() const;
			/// call this to announce the all data has been sent
			void done();
		};
		/// <summary>
		/// queued thread class that manages a named pipe 
		/// </summary>
		class CGV_API named_pipe_output_thread : public cgv::os::queued_output_thread
		{
		protected:
			/// based name of the 
			std::string pipe_name;
			/// pointer to the named pipe output stream
			nes::basic_pipe_ostream<char, std::char_traits<char>>* pipe_ptr = 0;
			/// creates pipe and waits for connection
			bool connect_to_child_process();
			/// write block to named pipe
			void write_block_to_pipe(const char* data, size_t count);
			/// closes named pipe
			void close();
		public:
			/// construct pipe output thread from pipe name, whether to use binary mode and wait time in ms used when queue is empty
			named_pipe_output_thread(const std::string& _pipe_name, bool is_binary = true, unsigned _ms_to_wait = 20);
			/// return path of pipe that can be used in command line arguments to child/client processes
			std::string get_pipe_path() const;
		};

		/// <summary>
		/// queued thread class that manages a child process connecting to its input pipe 
		/// </summary>
		class CGV_API pipe_output_thread : public cgv::os::queued_output_thread
		{
		protected:
			/// system command to be executed
			std::string cmd;
			/// file handle of stdin of child process
			FILE* fp = 0;
			/// result value of child process
			int result = -1;
			/// starts child process and connects to its stdin
			bool connect_to_child_process();
			/// write block to stdin pipe
			void write_block_to_pipe(const char* data, size_t count);
			/// closes stdin pipe and stores result value
			void close();
		public:
			/// construct pipe output thread from system command, whether to use binary mode and wait time in ms used when queue is empty
			pipe_output_thread(const std::string& _cmd, bool is_binary = true, unsigned _ms_to_wait = 20);
			/// return the result value returned from child process which is available only after termination of thread
			int get_result() const;
		};


		/// <summary>
		/// Base class for system command output pipe or named pipe threads including a queue of data blocks, a vector storing smaller data
		/// packages and a separate thread
		/// </summary>
		class CGV_API queued_input_thread : public cgv::os::thread
		{
		  protected:
			/// flag to mark end of data stream
			bool all_data_received = false;
			/// whether binary mode should be used
			bool is_binary;
			/// size of an indiviual package
			size_t package_size;
			/// index of the next package to be read
			size_t package_index;
			/// amount of packeges needed to form one data block
			size_t packages_per_block;
			/// mutex used to protect access to packages
			mutable std::mutex mutex_packages;
			/// mutex used to protect access to data_blocks
			mutable std::mutex mutex_data_blocks;
			/// allocated memory for smaller data packages
			char* packages;	
			/// deque to store data blocks that are formed by packages
			std::deque<std::vector<char>> data_blocks;

			/// time in milliseconds to wait while pipe is empty
			unsigned ms_to_wait;
			/// to be implemented in derived classes
			virtual bool connect_to_source() = 0;
			/// to be implemented in derived classes
			virtual size_t read_package_from_pipe(char* buffer, size_t package_size) = 0;
			/// combine the packages to a data block and push it to the queue
			virtual void move_packages_to_data_blocks();
			/// to be implemented in derived classes
			virtual void close() = 0;

		  public:
			/// construct queued input thread with binary mode flag and wait time in ms used when pipe is empty
			queued_input_thread(bool _is_binary, size_t _package_size,
								size_t _packeges_per_block, unsigned _ms_to_wait);

			/// continuously read from pipe and store in queue; if empty, wait in intervals of ms_to_wait
			/// milliseconds
			void run();
			/// if done() had not been called, read a data block from the queue; can fail if no data or done() was
			/// called
			bool pop_data_block(char* buffer);
			/// returns the number of blocks in the queue of not yet read data
			size_t get_nr_blocks() const;
			/// call this to indicate no more data will come
			void done();
		};

		class CGV_API named_pipe_input_thread : public cgv::os::queued_input_thread
		{
			protected:
			std::string pipe_name;
			nes::basic_pipe_istream<char, std::char_traits<char>>* pipe_ptr = 0;
			bool connect_to_source() override;
			size_t read_package_from_pipe(char* buffer,size_t package_size) override;
			void close() override;

		  public:
			named_pipe_input_thread(const std::string& _pipe_name, bool is_binary, size_t _package_size,
									size_t _packeges_per_block, unsigned _ms_to_wait);
			std::string get_pipe_path() const;
		};

		class CGV_API pipe_input_thread : public cgv::os::queued_input_thread
		{
		  protected:
			std::string cmd;
			FILE* fp = 0;

			int result = -1;

			bool connect_to_source() override;
			size_t read_package_from_pipe(char* buffer, size_t package_size) override;
			void close() override;

		  public:
			pipe_input_thread(const std::string& _cmd, bool is_binary, size_t _package_size, size_t _packages_per_block,
							  unsigned _ms_to_wait);
			int get_result() const;
		};
	}
}

#include <cgv/config/lib_end.h>