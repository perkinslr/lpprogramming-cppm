export module Subprocess;
import <unistd.h>;
import <sys/wait.h>;
import <fcntl.h>;
import <cstring>;
import <vector>;
import <string>;
import <span>;
import <memory>;
import <print>;
import <optional>;
import <array>;
import <source_location>;

import SyscallHelper;

using lpprogramming::SyscallHelper::Syscall;
#define SyscallOnce Syscall<false>

export
namespace lpprogramming::subprocess {
  struct PopenArgs {
    std::optional<int> stdin = std::nullopt;
    std::optional<int> stdout = std::nullopt;
    std::optional<int> stderr = std::nullopt;
  };
  constexpr int PIPE{-1};

  class Popen {
    static constexpr int NONE{-2};
    pid_t pid = -1;
    std::array<int, 3> fds{NONE, NONE, NONE};
    

  public:

    Popen(std::span<const std::string> args,
          const PopenArgs flags
          ) {
      auto stdin = flags.stdin;
      auto stdout = flags.stdout;
      auto stderr = flags.stderr;
      const std::string& program = args[0];
      std::array<int, 2> pipe_in {};
      std::array<int, 2> pipe_out {};
      std::array<int, 2> pipe_err {};

      if (stdin) {
        int in = stdin.value();
        if (in == PIPE) {
          Syscall(pipe, {"pipe() failed"}, pipe_in.data());
        }
      }
      if (stdout) {
        int out = stdout.value();
        if (out == PIPE) {
          Syscall(pipe, {"pipe() failed"}, pipe_out.data());
        }
      }
      if (stderr) {
        int err = stderr.value();
        if (err == PIPE) {
          Syscall(pipe, {"pipe() failed"}, pipe_err.data());
        }
      }

      pid = Syscall(fork, {"fork() failed"});

      if (pid == 0) {
        if (stdin) {
          int in = stdin.value();
          if (in == PIPE) {
            in = pipe_in[0];
            Syscall(close, {"close() failed"}, pipe_in[1]);
          }
          if (in != 0) {
            Syscall(dup2, {"dup2() failed"}, in, 0);
            Syscall(close, {"close() failed"}, in);
          }
        }
        if (stdout) {
          int out = stdout.value();
          if (out == PIPE) {
            out = pipe_out[1];
            Syscall(close, {"close() failed"}, pipe_out[0]);
          }
          if (out != 1) {
            Syscall(dup2, {"dup2() failed"}, out, 1);
            Syscall(close, {"close() failed"}, out);
          }
        }
        if (stderr) {
          int err = stderr.value();
          if (err == PIPE) {
            err = pipe_err[1];
            Syscall(close, {"close() failed"}, pipe_err[0]);
          }
          if (err != 2) {
            Syscall(dup2, {"dup2() failed"}, err, 2);
            Syscall(close, {"close() failed"}, err);
          }
        }

        std::vector<char*> argv;
        argv.reserve(args.size() + 1);
        for (const auto& arg : args) {
          argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execvp(program.c_str(), argv.data());
        _exit(127);
      }

      if (stdin) {
        int in = stdin.value();
        if (in == PIPE) {
          fds[0] = pipe_in[1];
          Syscall(close, {"close() failed"}, pipe_in[0]);
        }
      }
      if (stdout) {
        int out = stdout.value();
        if (out == PIPE) {
          fds[1] = pipe_out[0];
          Syscall(close, {"close() failed"}, pipe_out[1]);
        }
      }
      if (stderr) {
        int err = stderr.value();
        if (err == PIPE) {
          fds[2] = pipe_err[0];
          Syscall(close, {"close() failed"}, pipe_err[1]);
        }
      }
    }

    Popen(const Popen&) = delete;
    Popen& operator=(const Popen&) = delete;

    Popen(Popen&& other) = delete;

    Popen& operator=(Popen&& other) = delete;
    std::string read_stdout() {
      if (fds[1] > PIPE) {
        std::array<char, 128> buf{};
        ssize_t r = SyscallOnce(read, {"read() failed"}, fds[1], buf.data(), buf.size());
        return std::string{buf.begin(), buf.begin() + r};
      }
      else {
        throw std::runtime_error{"cannot read from non-pipe stdout"};
      }
    }

    std::string read_stdout_all() {
      std::println("fd: {}", fds[1]);
      if (fds[1] > PIPE) {
        std::string out{};
        std::array<char, 128> buf{};
        for (;;) {
          ssize_t r = Syscall(read, {"read() failed"}, fds[1], buf.data(), buf.size());
          if (!r) {
            return out;
          }
          out.append(buf.begin(), buf.begin() + r);
        }
        return out;
      }
      else {
        throw std::runtime_error{"cannot read from non-pipe stdout"};
      }
    }

    std::optional<int> poll() {
      int status;
      pid_t r = Syscall(waitpid, {"waitpid(WNOHANG) failed"}, pid, &status, WNOHANG);
      if (r > 0) {
        if (WIFEXITED(status)) {
          return WEXITSTATUS(status);
        }
        else if (WIFSIGNALED(status)) {
          return -WTERMSIG(status);
        }
        else {
          throw std::runtime_error{std::format("unrecognized exit: {}", status)};
        }
      }
      else {
        return std::nullopt;
      }
    }

    int wait() {
      int status;
      Syscall(waitpid, {"waitpid() failed"}, pid, &status, 0);
      if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
      }
      else if (WIFSIGNALED(status)) {
        return -WTERMSIG(status);
      }
      else {
        throw std::runtime_error{std::format("unrecognized exit: {}", status)};
      }
    }
  };

}
