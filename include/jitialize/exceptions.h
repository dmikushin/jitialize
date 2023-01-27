#ifndef EXCEPTIONS
#define EXCEPTIONS

#include <stdexcept>
#include <string>
#include <sstream>

namespace jitialize {
  struct exception
      : public std::runtime_error {
    exception(std::string const &Message, std::string const &Reason)
      : std::runtime_error(Message + Reason) {}
    virtual ~exception() = default;
  };
}

#define DefineJitializeException(Exception, Message) \
  struct Exception : public jitialize::exception { \
    Exception() : jitialize::exception(Message, "") {} \
    Exception(std::string const &Reason) : jitialize::exception(Message, Reason) {} \
    virtual ~Exception() = default; \
  }

#endif
