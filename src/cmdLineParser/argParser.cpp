/*
This file is part of KMailApplet.

KMailApplet is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KMailApplet is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with KMailApplet.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QtDebug>
#include <argParser.hpp>
#include <argp.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <limits>
#include <sstream>
#include <unistd.h>

/* global variables for the argp lib */
const char *argp_program_version = APPNAME;
const char *argp_program_bug_address = AUTHOR_EMAIL;
/* private static Members */
constexpr const char ArgParser::doc[];
constexpr const struct argp_option ArgParser::options[];
std::vector<std::string> ArgParser::argList; /* arguments list */
constexpr const struct argp ArgParser::argp; /* structure for argp parser */
constexpr const char ArgParser::delimiter;
const string ArgParser::mailKey = "mail";
string ArgParser::configFile = "";

using namespace std;
namespace fs = boost::filesystem;

/* Methods implementation */
error_t ArgParser::parseCmdLine(int argc, char **argv) {
  return argp_parse(&ArgParser::argp, argc, argv, 0, NULL, NULL);
}

void ArgParser::parseConfig() {
  if (configFile == "") {
    configFile = getenv("HOME");
    configFile.append("/.config/kmailapplet.conf");
  }
  if (fs::exists(configFile) && fs::is_regular_file(configFile)) {
    string line;
    ifstream file(configFile);
    while (getline(file, line)) {
      boost::trim(line);
      // skip comments
      if (line.front() == '#') {
        continue;
      }
      istringstream linestream(line);
      string key;
      if (getline(linestream, key, ArgParser::delimiter)) {
        if (boost::iequals(key, ArgParser::mailKey)) {
          line.erase(0, line.find_first_of(ArgParser::delimiter) + 1);
          // expand ~ in $HOME
          if (line.front() == '~') {
            line.replace(line.cbegin(), line.cbegin() + 1, getenv("HOME"));
          }
          ArgParser::argList.push_back(line);
        }
      }
      if (file.fail() || linestream.fail()) {
        qWarning() << "Error: configuration file badly formatted";
        break;
      }
    }
  } else {
    qWarning() << "Error: configuration file does not exist";
  }
}

error_t ArgParser::parser_function(int key, char *arg,
                                   struct argp_state *state) {
  switch (key) {
  case 'm':
    if (fs::exists(arg) && fs::is_directory(arg) && !fs::is_empty(arg)) {
      ArgParser::argList.push_back(arg);
    } else {
      qWarning() << arg << " : no such file or directory";
      return 1;
    }
    break;
  case 'c':
    if (fs::exists(arg) && fs::is_regular_file(arg) &&
        ArgParser::configFile == "") {
      ArgParser::configFile = arg;
    } else {
      qWarning() << arg << " : no such file";
      return 1;
    }
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}
