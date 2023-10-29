#include <csignal>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <syslog.h>
#include <sys/stat.h>
#include <thread>

#include "Reminder.h"

Reminder& Reminder::getInstance() {
    static Reminder instance;
    return instance;
}

void Reminder::init(std::string configFile) {
    openlog("Reminder", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "Logger successfully opened");
    syslog(LOG_INFO, "Daemon initialization");

    syslog(LOG_INFO, "Getting config absolute path");
    char buf[PATH_MAX];
    getcwd(buf, sizeof(buf));
    m_configFilePath = buf;
    m_configFilePath += "/" + configFile;

    checkPid();
    toDaemon();
    writePid();

    syslog(LOG_INFO, "Seting signal handler");
    std::signal(SIGHUP, signalHandler);
    std::signal(SIGTERM, signalHandler);

    loadConfig();
    syslog(LOG_INFO, "Daemon successfully initialized");
}

void Reminder::checkPid() {
    syslog(LOG_INFO, "Checking is reminder already running");
    std::ifstream pidFile(PID_FILEPATH);
    
    syslog(LOG_INFO, "Checking if reminder pid file opened");
    if (!pidFile.is_open()) {
        syslog(LOG_INFO, "No running reminder daemon found");
    }
    syslog(LOG_INFO, "Running reminder found");
    
    syslog(LOG_INFO, "Checking if found reminder still exists");
    pid_t pid{};
    if (pidFile >> pid && !kill(pid, 0)) {
        syslog(LOG_WARNING, "Stopping a previously running daemon (pid: %i)", pid);
        kill(pid, SIGTERM);
    }
}

void Reminder::toDaemon() {
    syslog(LOG_INFO, "Starting daemonization");

    syslog(LOG_INFO, "Forking process");
    pid_t pid{ fork() };
    if (pid < 0) {
        syslog(LOG_ERR, "Forking error");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        syslog(LOG_INFO, "Successfully forked (parent)");
        exit(EXIT_SUCCESS);
    }
    syslog(LOG_INFO, "Successfully forked (child)");
        
    syslog(LOG_INFO, "Setting process mask");
    umask(0);

    syslog(LOG_INFO, "Setting group");
    if (setsid() < 0) {
        syslog(LOG_ERR, "Setting group error");
        exit(EXIT_FAILURE);
    }
    
    syslog(LOG_INFO, "Changing directory");
    if (chdir("/") < 0) {
        syslog(LOG_ERR, "Changing directory error");
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Successfully daemonized");
}

void Reminder::writePid() {
    syslog(LOG_INFO, "Writing pid");

    syslog(LOG_INFO, "Checking is reminder pid file opened");
    std::ofstream pidFile(PID_FILEPATH.c_str());
    if (!pidFile.is_open()) {
        syslog(LOG_ERR, "Writing pid error");
        exit(EXIT_FAILURE);
    }
    
    pidFile << getpid();
    syslog(LOG_INFO, "Pid successfully written");
}

void Reminder::signalHandler(int sig) {
    syslog(LOG_INFO, "Processing signal: %i", sig);

    switch (sig) {
    case SIGHUP:
        syslog(LOG_INFO, "Reloading config");
        Reminder::getInstance().loadConfig();
        break;
    case SIGTERM:
        syslog(LOG_INFO, "Terminating process");
        Reminder::getInstance().terminate();
        break;
    }
}

void Reminder::loadConfig() {
    syslog(LOG_INFO, "Loading config");

    syslog(LOG_INFO, "Clearing current event list");
    m_events.clear();

    syslog(LOG_INFO, "Checking is config file opened");
    std::ifstream configFile(m_configFilePath);
    if (!configFile.is_open()) {
        syslog(LOG_ERR, "Config file already opened");
        m_isTerminated = true;
        return;
    }

    syslog(LOG_INFO, "Processing config file");
    std::string str;
    while (std::getline(configFile, str)) {
        syslog(LOG_INFO, "Processing config file line: %s", str.c_str());
        if (std::regex_match(str, EVENT_REGEX)) {
            syslog(LOG_INFO, "String matches event regex");
            parseEvent(str);
        }
        else {
            syslog(LOG_WARNING, "Failed to parse string. Ignoring");
        }
    }
    syslog(LOG_INFO, "Config file processing finished");

    if (m_events.empty())
        syslog(LOG_WARNING, "No reminder events found");
    else
        syslog(LOG_INFO, "%li reminder events found", m_events.size());

    syslog(LOG_INFO, "Config loaded");
}

void Reminder::parseEvent(std::string& line) {
    syslog(LOG_INFO, "Parsing event string");

    std::smatch match;
    std::regex_search(line, match, EVENT_REGEX);
    
    syslog(LOG_INFO, "Parsing event time");
    std::tm tmEventTime;
    if ((std::istringstream(match[1]) >> std::get_time(&tmEventTime, "%d/%m/%Y %T")).fail()) {
        syslog(LOG_WARNING, "Failed to parse time, event will be ignored");
        return;
    }
    auto eventTime = std::chrono::system_clock::from_time_t(std::mktime(&tmEventTime));
    syslog(LOG_INFO, "Event time parsed");

    syslog(LOG_INFO, "Parsing event repetition");
    int seconds{};
    if (match[2] != "") {
        if (match[2] == "-m")
            seconds = 60;
        else if (match[2] == "-h")
            seconds = 60 * 60;
        else if (match[2] == "-d")
            seconds = 24 * 60 * 60;
        else if (match[2] == "-w")
            seconds = 7 * 24 * 60 * 60;
        else 
            syslog(LOG_WARNING, "Failed to parse repeat interval, ignoring");
    }
    syslog(LOG_INFO, "Event repetition %s",
            seconds ? "parsed" : "isn't specified"
    );
    std::chrono::seconds repeat{ seconds };

    syslog(LOG_INFO, "Processing event time");
    auto now = std::chrono::system_clock::now();
    if (eventTime < now) {
        if (repeat == std::chrono::seconds(0)) {
            syslog(LOG_WARNING, "Event time has passed, event will be ignored");
            return;
        }

        auto delta = now - eventTime;
        auto step = delta / repeat + (delta % repeat == std::chrono::seconds(0) ? 0 : 1);
        eventTime += std::chrono::duration_cast<std::chrono::seconds>(step * repeat);
    }
    syslog(LOG_INFO, "Event time processed");

    m_events.push_back({ eventTime, repeat, match[3] });
    syslog(LOG_INFO, "Event added to list");
}

void Reminder::terminate() {
    syslog(LOG_WARNING, "Terminating process");
    m_isTerminated = true;

    syslog(LOG_INFO, "Closing logger");
    closelog();
}

void Reminder::run() {
    syslog(LOG_INFO, "Reminder daemon is working");

    while (!m_isTerminated) {
        auto now = std::chrono::system_clock::now();
        auto next = now + SLEEP_TIME;
        
        for (auto e = m_events.begin(); e != m_events.end(); ++e) {
            if (now < e->time) {
                next = std::min(next, e->time);
                continue;
            }


            // syslog(LOG_INFO, "%s", e->text.c_str());
            std::string text = "gnome-terminal -- bash -c \"echo '" + e->text + "'; read n\"";
            system(text.c_str());

            if (e->repeat != std::chrono::seconds(0))
              e->time += e->repeat;
            else
              m_events.erase(e--);
        }

        std::this_thread::sleep_until(next);
    }
}
