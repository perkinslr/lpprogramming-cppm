export module SyscallHelper;
import <errno.h>;
import <exception>;
import <format>;
import <system_error>;
import <functional>;
import <system_error>;
import <source_location>;


export
namespace lpprogramming::SyscallHelper {
  struct failure_message {
    const std::string_view message;
    const std::source_location loc;
    constexpr failure_message(const std::string_view m, const std::source_location sl = std::source_location::current()) : message(m), loc(sl) {}
  };

  template<bool again = true, typename Function, typename... Args>
  inline auto Syscall(Function &function, const failure_message message, Args&&... args) {
    using result = std::invoke_result<Function, Args...>::type;
    result res;
    do {
      errno = 0;
      res = function(args...);
      if constexpr (!again) {
        if (errno == EAGAIN) {
          throw std::system_error({EAGAIN, std::system_category()}, std::format("EAGAIN: {} at {}:{}", message.message, message.loc.file_name(), message.loc.line()));
        }
      }
    } while (errno == EAGAIN || errno == EINTR);
    if (res < 0) {
      throw std::system_error({errno, std::system_category()}, std::format("{} at {}:{}", message.message, message.loc.file_name(), message.loc.line()));
    }
    return res;
  }

}

