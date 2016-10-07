#include <curses.h>

#include <string>

class Window {
 private:
  WINDOW* win_;

 public:
  Window(int x, int y, int width, int height) : win_(newwin(height, width, y, x)) {
    nodelay(win_, TRUE);
  }

  ~Window() { delwin(win_); }

  void refresh() { wrefresh(win_); }

  void enable_scroll() { scrollok(win_, TRUE); }
  void disable_scroll() { scrollok(win_, FALSE); }

  void disable_delay() { nodelay(win_, TRUE); }

  void add_border() { wborder(win_, 0, 0, 0, 0, 0, 0, 0, 0); }

  void move_cursor(int x, int y) { wmove(win_, y, x); }

  char get_ch() { return wgetch(win_); }

  void set_cursor_pos(int x, int y) { wmove(win_, y, x); }

  void write_ch(int x, int y, char c) { mvwaddch(win_, y, x, c); }

  void write_string(int x, int y, const std::string& str) {
    for (std::size_t i = 0; i < str.length(); i++) {
      write_ch(x + i, y, str[i]);
    }
    refresh();
  }

  void add_line(const std::string& str) {
    wprintw(win_, str.c_str());
    wprintw(win_, "\n");
    refresh();
  }
};
