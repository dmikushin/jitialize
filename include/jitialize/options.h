#ifndef OPTIONS
#define OPTIONS

#include <jitialize/runtime/Context.h>

#define JITIALIZE_NEW_OPTION_STRUCT(Name) \
  struct Name; \
  template<> struct is_option<Name> { \
    static constexpr bool value = true; }; \
  struct Name

#define JITIALIZE_HANDLE_OPTION_STRUCT(Name, Ctx) \
  void handle(jitialize::Context &Ctx) const


namespace jitialize {
namespace options{

  template<class T>
  struct is_option {
    static constexpr bool value = false;
  };

  JITIALIZE_NEW_OPTION_STRUCT(opt_level)
    : public std::pair<unsigned, unsigned> {

    opt_level(unsigned OptLevel, unsigned OptSize)
               : std::pair<unsigned, unsigned>(OptLevel,OptSize) {}

    JITIALIZE_HANDLE_OPTION_STRUCT(opt_level, C) {
      C.setOptLevel(first, second);
    }
  };

  // option used for writing the ir to a file, useful for debugging
  JITIALIZE_NEW_OPTION_STRUCT(dump_ir) {
    dump_ir(std::string const &file)
               : file_(file) {}

    JITIALIZE_HANDLE_OPTION_STRUCT(dump_ir, C) {
      C.setDebugFile(file_);
    }

    private:
    std::string file_;
  };
}
}

#endif // OPTIONS
