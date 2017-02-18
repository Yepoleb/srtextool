#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "args.hxx"

#include "../path.hpp"
#include "shared.hpp"

static const char* HELP_MAIN =
R"(
SR Textool 1.0.2

Tool for converting DDS textures to and from the Volition bitmap format.

Usage: % [options] <command> [<args...>]

Options:

  -h, --help                        Display this help menu
  command                           Command to execute
  args                              Arguments for the command

  "--" can be used to terminate flag options and force all following
  arguments to be treated as positional options

Commands:

  x: Extract textures
  a: Add or update textures
  l: List header fields and entries
  d: Delete textures
  m: Modify texture properties
  c: Check texture for errors

)";

int main(int argc, char** argv)
{
    std::unordered_map<std::string, commandtype> cmdmap = {
        {"x", cmd_extract},
        {"a", cmd_add},
        {"l", cmd_list},
        {"d", cmd_delete},
        {"m", cmd_modify},
        {"c", cmd_check}
    };

    std::string progname = path::basename(argv[0]);
    std::vector<std::string> cmdargs(argv + 1, argv + argc);

    args::ArgumentParser parser("");
    args::HelpFlag help(parser, "help", "", {'h', "help"});
    args::MapPositional<std::string, commandtype> command_arg(parser, "command", "", cmdmap);
    command_arg.KickOut(true);

    try {
        auto next = parser.ParseArgs(cmdargs);
        if (command_arg) {
            return args::get(command_arg)(progname, next, std::end(cmdargs));
        } else {
            std::cout << help_format(HELP_MAIN, progname);
        }
    } catch (args::Help) {
        std::cout << help_format(HELP_MAIN, progname);
        return 0;
    } catch (const args::Error& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << help_format(HELP_MAIN, progname);
        return 1;
    }

    return 0;
}
