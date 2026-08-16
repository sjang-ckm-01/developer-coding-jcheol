#include <cstdio>
#include <format>

template <class... Args>
void println(std::format_string<Args...> fmt, Args&&... args) {
  std::string s = std::format(fmt, std::forward<Args>(args)...);
  s.push_back('\n');
  std::fwrite(s.data(), 1, s.size(), stdout);
}

int main() {
  println("hello:{}", 123);

  return 0;
}
