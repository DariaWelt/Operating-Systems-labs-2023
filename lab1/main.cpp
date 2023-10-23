#include "daemon.h"
#include <iostream>

int main(int argc, char *argv[]) {
  std::string config_path;

  switch (argc) {
  case 1:
    config_path = "config.txt";
    break;
  case 2:
    config_path = argv[1];
    break;
  default:
    std::cout << "Too many arguments" << std::endl;
    return 1;
  }

  Daemon &daemon = Daemon::getInstance(config_path);
  daemon.run();
}