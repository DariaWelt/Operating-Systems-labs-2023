#pragma once
#include "config.h"
#include <filesystem>

class Daemon {
public:
  static Daemon &getInstance(const std::string &configPath);
  void run();
  Daemon() = delete;
  Daemon(const Daemon &) = delete;
  Daemon(Daemon &&) = delete;

protected:
  static void handleSignals(int sig);
  void closeIfExists();
  void detachFromProcess() const;

private:
  Daemon(const std::string &configPath);

  static std::string mPathToConfig;
  static Config mConfig;
  unsigned int mTimeInterval = 60;
  std::string mProcessName = "daemon";
  std::string mPidFile = std::filesystem::absolute("daemon.pid");
};
