#if defined(WIN32)

// TODO: make OS implementation inner

namespace trajectory {
namespace util {
	namespace mouse {
		bool set_position(int x, int y);

		void hold_left();
		void release_left();
		void left_click();
		void double_left_click();

		void right_click();

		void hold_middle();
		void release_middle();
		void middle_click();

		void scroll(int n);
		void scroll_up(unsigned int n);
		void scroll_down(unsigned int n);

	} // namespace mouse
} // namespace util
} // namespace trajectory

#endif