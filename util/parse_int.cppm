export module parse_int;

import <ranges>;
import <string_view>;
import <stdexcept>;
import <format>;
import <algorithm>;
import <cstdint>;

export
namespace lpprogramming::util {
  constexpr int parse_int(std::string_view s) {
    int sign = 1;
    bool past_prefix = false;

    auto digits = s | std::views::transform
      ([&](char c) constexpr mutable {
        switch(c) {
        case '-':
          sign = -sign;
        case '+':
          if (past_prefix) {
            throw std::runtime_error(std::format("invalid digit past prefix: {}", s));
          }
        case ' ':
        case '_':
        case '\'':
          return -1;
        }
        if (c < '0' || c > '9') {
          throw std::runtime_error(std::format("invalid digit: {}", c));
        }
        past_prefix = true;
        return c - '0';
      });

    int64_t value = std::ranges::fold_left
      (digits, 0,
       [](int64_t acc, const int d) {
         if (d == -1) {
           return acc;
         }
         
         acc = acc * 10 + d;
         if (acc > std::numeric_limits<int>::max()) {
           throw std::runtime_error("Number too large to fit in int");
         }
         return acc;

       });
    return sign * static_cast<int>(value);
  }
}
