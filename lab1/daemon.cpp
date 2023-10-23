#include "daemon.h"
#include "config.h"
#include <csignal>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

namespace fs = std::filesystem;

std::string Daemon::mPathToConfig;
Config Daemon::mConfig;

void Daemon::run() {
  closeIfExists();
  detachFromProcess();
  syslog(LOG_NOTICE, "Daemon started");

  while (true) {
    std::vector<ConfigRow> rows = mConfig.getRows();

    for (const auto &row : rows) {
      for (const auto &file : fs::directory_iterator(row.dir2))
        fs::remove(file);

      fs::create_directory(row.dir2 / row.subfolder);
      fs::create_directory(row.dir2 / "OTHERS");

      for (const auto &file : fs::directory_iterator(row.dir1)) {
        if (file.path().extension() == row.ext)
          fs::copy(file.path(), row.dir2 / row.subfolder);
        else
          fs::copy(file.path(), row.dir2 / "OTHERS");
      }
    }

    sleep(mTimeInterval);
  }
}

void Daemon::closeIfExists() {
  std::ifstream f(mPidFile);
  int pid;
  f >> pid;
  if (fs::exists("/proc/" + std::to_string(pid)))
    kill(pid, SIGTERM);
}

void Daemon::handleSignals(int sig) {
  switch (sig) {
  case SIGHUP:
    mConfig.readConfig(mPathToConfig);
    break;

  case SIGTERM:
    syslog(LOG_NOTICE, "Daemon closed");
    closelog();
    exit(EXIT_SUCCESS);

  default:
    break;
  }
}

Daemon &Daemon::getInstance(const std::string &configPath) {
  static Daemon instance(configPath);
  return instance;
}

Daemon::Daemon(const std::string &configPath) {
  mPathToConfig = fs::absolute(configPath);
  mConfig = Config(mPathToConfig);
}

void Daemon::detachFromProcess() const {
  pid_t pid = fork();

  if (pid != 0)
    exit(EXIT_FAILURE);
  if (setsid() < 0)
    exit(EXIT_FAILURE);

  std::signal(SIGHUP, handleSignals);
  std::signal(SIGTERM, handleSignals);

  pid = fork();

  if (fork() != 0)
    exit(EXIT_FAILURE);
  umask(0);
  if (chdir("/") != 0)
    exit(EXIT_FAILURE);

  for (long x = sysconf(_SC_OPEN_MAX); x >= 0; --x)
    close(x);
  openlog(mProcessName.c_str(), LOG_PID, LOG_DAEMON);

  std::ofstream f(mPidFile, std::ios_base::trunc);
  f << getpid();
}
