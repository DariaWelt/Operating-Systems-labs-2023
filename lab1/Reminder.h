#pragma once

#include <chrono>
#include <regex>
#include <string>
#include <vector>

// singleton reminder daemon class
class Reminder {
    // regex for reading events from config
    const std::regex EVENT_REGEX{
        R"(^\s*add_event\s+(\S+\s+\S+)\s+(-m|-h|-d|-w)*\s*(.+)\s*)"
    };
    
    // filepath for pid file
    const std::string PID_FILEPATH{ "/var/run/reminder_daemon.pid" };
    
    // sleep time between checking events
    const std::chrono::seconds SLEEP_TIME{ std::chrono::seconds(10) };
    
    struct Event {
        std::chrono::system_clock::time_point time{};   // event time
        std::chrono::seconds repeat{};  // repetition time (minute/hour/day/week)
        std::string text{}; // text to remind
    };

    bool m_isTerminated{};  // is daemon terminated
    std::string m_configFilePath{}; // filepath to config file
    std::vector<Event> m_events{};  // list of actual events

    // singleton
    Reminder() {}
    Reminder(const Reminder&) = delete;
    Reminder& operator=(const Reminder&) = delete;

  public:
    static Reminder& getInstance(); // get singleton instance

    void init(std::string configPath); // init with config    
    void run(); // start working

  private:
    void checkPid();    // check running daemon
    void toDaemon();    // turn process into daemon
    void writePid();    // write new pid to pid file
    static void signalHandler(int sig); // signal handler for SIGHUP and SIGTERM

    void loadConfig();  // read events from config
    void parseEvent(std::string& line); // parse string with event

    void terminate();   // terminate process
};