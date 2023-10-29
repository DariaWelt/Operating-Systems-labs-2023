#include <iostream>

#include "Reminder.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Invalid args: specify relative filepath to single config file\n";
    return EXIT_FAILURE;
  }

  Reminder::getInstance().init(argv[1]);  // initialize reminder with config from args
  Reminder::getInstance().run();  // start reminder running

  return EXIT_SUCCESS;
}
