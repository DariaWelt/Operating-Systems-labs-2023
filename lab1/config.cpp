#include "config.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <syslog.h>
#include <vector>

namespace fs = std::filesystem;

Config::Config(const std::string &path) { readConfig(path); }

void Config::readConfig(const std::string &path) {
  mRows.clear();

  std::ifstream f(path);
  fs::path dir1;
  fs::path dir2;
  std::string ext;
  std::string subfolder;

  while (f >> dir1 >> dir2 >> ext >> subfolder) {
    mRows.push_back(ConfigRow{dir1, dir2, ext, subfolder});
  }

  if (f.eof())
    syslog(LOG_NOTICE, "Incorrect config");
}

std::vector<ConfigRow> Config::getRows() const { return mRows; }
