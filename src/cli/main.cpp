#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "args.hxx"

#include "../path.hpp"
#include "shared.hpp"

using commandtype = std::function<int(const std::string&, std::vector<std::string>::const_iterator, std::vector<std::string>::const_iterator)>;

// Forward declare commands so I don't need a 10 lines header file

int cmd_add(std::string progname, std::vector<std::string>::const_iterator beginargs, std::vector<std::string>::const_iterator endargs);
int cmd_check(std::string progname, std::vector<std::string>::const_iterator beginargs, std::vector<std::string>::const_iterator endargs);
int cmd_delete(std::string progname, std::vector<std::string>::const_iterator beginargs, std::vector<std::string>::const_iterator endargs);
int cmd_extract(std::string progname, std::vector<std::string>::const_iterator beginargs, std::vector<std::string>::const_iterator endargs);
int cmd_list(std::string progname, std::vector<std::string>::const_iterator beginargs, std::vector<std::string>::const_iterator endargs);

static const char* HELP_MAIN =
R"(
SR Textool 1.0.0

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
  c: Check texture for errors

)";

int main(int argc, char** argv)
{
    std::unordered_map<std::string, commandtype> cmdmap = {
        {"x", cmd_extract},
        {"a", cmd_add},
        {"l", cmd_list},
        {"d", cmd_delete},
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
