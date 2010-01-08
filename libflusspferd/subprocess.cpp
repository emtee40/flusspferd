/*
The MIT License

Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
                                       http://flusspferd.org/contributors.txt)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "flusspferd/subprocess.hpp"
#include "flusspferd/exception.hpp"

#ifdef WIN32

void flusspferd::load_subprocess_module(object &ctx) {
  throw flusspferd::exception("Subprocess Module Not Yet Supported On Windows", "Error");
}

#else // POSIX/UNIX

#include "flusspferd/io/stream.hpp"
#include "flusspferd/create/function.hpp"
#include "flusspferd/property_iterator.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <vector>
#include <cstdio>

#include "flusspferd/value_io.hpp" // DEBUG
#include <iostream> // DEBUG

#if defined(__APPLE__)
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron())
#else
extern char** environ;
# endif

using namespace flusspferd;

FLUSSPFERD_CLASS_DESCRIPTION(
  subprocess,
  (constructor_name, "subprocess")
  (full_name, "subprocess.subprocess")
  (constructible, false)
  (methods,
   ("poll", bind, poll)
   ("wait", bind, wait)
   ("communicate", bind, communicate)
   ("sendSignal", bind, send_signal)
   ("terminate", bind, terminate)
   ("kill", bind, kill))
  (properties,
   ("pid", getter, get_pid)
   ("returncode", getter, get_returncode)
   ("stdin", getter, get_stdin)
   ("stderr", getter, get_stderr)
   ("stdout", getter, get_stdout)))
{
  pid_t pid;
  int returncode;

  value wait_impl(bool poll = false) {
    if(pid != 0) {
      int status;
      pid_t const ret = waitpid(pid, &status, poll ? WNOHANG : 0);
      int const errno_ = errno;
      if(ret == -1) {
        throw flusspferd::exception(std::string("waitpid: ") + std::strerror(errno_));
      }
      else if(ret == 0) {
        return value(object());
      }
      else {
        if(WIFSIGNALED(status)) {
          returncode = -WTERMSIG(status);
          pid = 0;
          return value(returncode);
        }
        else if(WIFEXITED(status)) {
          returncode = WEXITSTATUS(status);
          pid = 0;
          return value(returncode);
        }
      }
    }
    return value(object());
  }
 public:
  value poll() {
    return wait_impl(true);
  }
  value wait() {
    return wait_impl(false);
  }
  object communicate(boost::optional<std::string> const &in) {
    (void)in;
    return object();
  }
  void send_signal(int sig) {
    if(pid != 0) {
      if(::kill(pid, sig) == -1) {
        int const errno_ = errno;
        throw flusspferd::exception(std::string("kill: ") + std::strerror(errno_));
      }
    }
  }
  void terminate() {
    send_signal(SIGTERM);
  }
  void kill() {
    send_signal(SIGKILL);
  }
  int get_pid() { return static_cast<int>(pid); }
  value get_returncode() {
    if(pid != 0) {
      return value(returncode);
    }
    else {
      return value(object());
    }
  }
  /*io::stream*/object get_stdin() { return object(); }
  object get_stderr() { return object(); }
  object get_stdout() { return object(); }
};

namespace {
  void do_pipe(int pipefd[2]) {
    if(::pipe(pipefd) == -1) {
      int const errno_ = errno;
      throw flusspferd::exception(std::string("pipe: ") + std::strerror(errno_));
    }
  }

  subprocess do_popen(char const *cmd,
                      std::vector<char const*> const &args,
                      std::vector<char const*> const &env,
                      bool stdin_, bool stdout_, bool stderr_,
                      char const *cwd = 0x0)
  {
    assert(cmd);
    assert(!args.empty() && args.back() == 0x0);
    assert(env.empty() || env.back() == 0x0);
    int stdinp[2];
    if(stdin_) {
      do_pipe(stdinp);
    }
    int stdoutp[2];
    if(stdout_) {
      do_pipe(stdoutp);
    }
    int stderrp[2];
    if(stderr_) {
      do_pipe(stderrp);
    }

    pid_t pid = vfork();
    if(pid == -1) {
      int const errno_ = errno;
      throw flusspferd::exception(std::string("vfork: ") + std::strerror(errno_));
    }
    else if(pid == 0) {
      if(stdin_) {
        close(stdinp[1]);
        if(stdinp[0] != STDIN_FILENO) {
          dup2(stdinp[0], STDIN_FILENO);
          close(stdinp[0]);
        }
      }
      if(stdout_) {
        int tp1 = stdoutp[1];
        close(stdoutp[0]);
        if(tp1 != STDOUT_FILENO) {
          dup2(tp1, STDOUT_FILENO);
          close(tp1);
          tp1 = STDOUT_FILENO;
        }
      }
      if(stderr_) {
        int tp1 = stderrp[1];
        close(stderrp[0]);
        if(tp1 != STDERR_FILENO) {
          dup2(tp1, STDERR_FILENO);
          close(tp1);
          tp1 = STDERR_FILENO;
        }
      }
      if(cwd) {
        //chdir(cwd); // TODO check if this is allowed in vfork!
      }
      execve(cmd, const_cast<char *const*>(&args[0]),
             env.empty() ? environ : const_cast<char *const*>(&env[0]));
      // TODO This shouldn't use environ but system.env instead! (BUG)
      _exit(127);
    }

    // ...
  }
}

/*
call style:

"w","r"

popen("foo", "w"); // popen(3)

popen(["foo", "arg0", "arg1"], "r"); // similar to popen(3) but uses execve instead of shell

popen(obj);
obj = {
  args : Array containing the Arguments (args[0] is used as executable if .executable is null!,
  executable : optString - The name of the executable,
  shell : optBoolean - true -> use shell to invoke program or false -> call execve? (default: false)
  cwd : optString - Working Directory
  env : optEnumeratableObject containing the environment (default: system.env)
  stdin : optBoolean - true -> open pipe for stdin (default: true)
  stdout : optBoolean
  stderr : optBoolean
};

 */

void Popen(flusspferd::call_context &x) {
  if(x.arg.size() == 1) {
    if(!x.arg.front().is_object()) {
      throw flusspferd::exception("popen: expects object as parameter", "TypeError");
    }
  }
  else if(x.arg.size() == 2) {
    if(x.arg[0].is_string() && x.arg[1].is_string()) {
      std::vector<char const*> args;
      args.push_back("sh");
      args.push_back("-c");
      args.push_back(x.arg[0].get_string().c_str());

      std::vector<char const*> env;

      bool in = false;
      if(x.arg[1].get_string() == "w") {
        in = true;
      }
      else if(!(x.arg[1].get_string() == "r")) {
        throw flusspferd::exception("popen: second parameter should be \"r\" or \"w\"");
      }

      x.result = do_popen("sh", args, env, in, !in, false);
      return;
    }
    else if(x.arg[0].is_object() && x.arg[0].get_object().is_array() && x.arg[1].is_string()) {
    }
    else {
    }
  }
  else {
    throw flusspferd::exception("popen: wrong number of parameters");
  }
}

void flusspferd::load_subprocess_module(object &ctx) {
  object exports = ctx.get_property_object("exports");

  flusspferd::create<flusspferd::function>(
    "popen", &Popen,
    param::_container = exports);
  load_class<subprocess>(exports);
  exports.define_properties(read_only_property | permanent_property)
    ("SIGABRT", value(static_cast<int>(SIGABRT))) // Process abort signal. (A)
    ("SIGALRM", value(static_cast<int>(SIGALRM))) // Alarm clock. (T)
    ("SIGBUS", value(static_cast<int>(SIGBUS))) // Access to an undefined portion of a memory object. (A)
    ("SIGCHLD", value(static_cast<int>(SIGCHLD))) // Child process terminated, stopped, or continued. (I)
    ("SIGCONT", value(static_cast<int>(SIGCONT))) // Continue executing, if stopped. (C)
    ("SIGFPE", value(static_cast<int>(SIGFPE))) // Erroneous arithmetic operation. (A)
    ("SIGHUP", value(static_cast<int>(SIGHUP))) // Hangup. (T)
    ("SIGILL", value(static_cast<int>(SIGILL))) // Illegal instruction. (A)
    ("SIGINT", value(static_cast<int>(SIGINT))) // Terminal interrupt signal. (T)
    ("SIGKILL", value(static_cast<int>(SIGKILL))) // Kill (cannot be caught or ignored). (T)
    ("SIGPIPE", value(static_cast<int>(SIGPIPE))) // Write on a pipe with no one to read it. (T)
    ("SIGQUIT", value(static_cast<int>(SIGQUIT))) // Terminal quit signal. (A)
    ("SIGSEGV", value(static_cast<int>(SIGSEGV))) // Invalid memory reference. (A)
    ("SIGSTOP", value(static_cast<int>(SIGSTOP))) // Stop executing (cannot be caught or ignored). (S)
    ("SIGTERM", value(static_cast<int>(SIGTERM))) // Termination signal. (T)
    ("SIGTSTP", value(static_cast<int>(SIGTSTP))) // Terminal stop signal. (S)
    ("SIGTTIN", value(static_cast<int>(SIGTTIN))) // Background process attempting read. (S)
    ("SIGTTOU", value(static_cast<int>(SIGTTOU))) // Background process attempting write. (S)
    ("SIGUSR1", value(static_cast<int>(SIGUSR1))) // User-defined signal 1. (T)
    ("SIGUSR2", value(static_cast<int>(SIGUSR2))) // User-defined signal 2. (T)
    ("SIGPOLL", value(static_cast<int>(SIGPOLL))) // Pollable event. (T)
    ("SIGPROF", value(static_cast<int>(SIGPROF))) // Profiling timer expired. (T)
    ("SIGSYS", value(static_cast<int>(SIGSYS))) // Bad system call. (A)
    ("SIGTRAP", value(static_cast<int>(SIGTRAP))) // Trace/breakpoint trap. (A)
    ("SIGURG", value(static_cast<int>(SIGURG))) // High bandwidth data is available at a socket. (I)
    ("SIGVTALRM", value(static_cast<int>(SIGVTALRM))) // Virtual timer expired. (T)
    ("SIGXCPU", value(static_cast<int>(SIGXCPU))) // CPU time limit exceeded. (A)
    ("SIGXFSZ", value(static_cast<int>(SIGXFSZ))); // File size limit exceeded. (A)
}

#endif
