#pragma once
#include <filesystem>
#include <string>
#include <vector>

struct ConfigRow {
  std::filesystem::path dir1;
  std::filesystem::path dir2;
  std::string ext;
  std::string subfolder;
};

class Config {
public:
  Config() = default;
  Config(const std::string &path);
  void readConfig(const std::string &path);
  std::vector<ConfigRow> getRows() const;

private:
  std::vector<ConfigRow> mRows;
};
