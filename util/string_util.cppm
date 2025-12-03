export module string_util;
import <string>;
import <algorithm>;
import <ranges>;
import <functional>;
import <vector>;

export
namespace lpprogramming::util {
  inline constexpr std::string string_concat(const std::vector<std::string> &in) {
    if (in.size() < 1) {
      return "";
    }
    return std::ranges::fold_left(std::views::drop(in, 1), in[0], std::plus<>());
  }
  inline constexpr std::string iter_replace(const std::string_view query, const auto& sep, const auto& func) {
    auto parts = std::views::split(query, std::string{sep}) 
      | std::ranges::to<std::vector<std::string>>();
    
    if (parts.size() == 0) {
      return std::string{""};
    }

    return std::ranges::fold_left
      (
       parts | std::ranges::views::drop(1),
       parts[0],
       func
       );
    
  }
}
