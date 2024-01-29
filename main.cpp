// Copyright 2023 Bikash Shrestha
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <utility>
#include <boost/date_time/posix_time/posix_time.hpp>

using std::string;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::istringstream;
using std::pair;
using std::cerr;
using std::endl;
using std::regex;
using std::regex_search;
using std::smatch;
using std::make_pair;
using boost::posix_time::ptime;
using boost::posix_time::time_duration;
using boost::posix_time::time_from_string;
using boost::posix_time::to_simple_string;

// Function prototypes
void processLogFile(const string& fileName);
pair<ptime, ptime> findBootTimes(const string& logLine, const regex& pattern);
bool isBootComplete(const string& logLine);
bool isBootStart(const string& logLine);
string formatDateTime(const ptime& dateTime);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <log_file1> " << endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        processLogFile(argv[i]);
    }

    return 0;
}

// Process a log file and write the report to a file
void processLogFile(const string& fileName) {
    ifstream logFile(fileName);
    if (!logFile.is_open()) {
        cerr << "Error opening file: " << fileName << endl;
        return;
    }

    // Read entire file into a string
    stringstream buffer;
    buffer << logFile.rdbuf();
    string content = buffer.str();
    logFile.close();

    // Pre-compile the regular expression
    regex pattern(R"(\b(\d{4}-\d{2}-\d{2})\s(\d{2}:\d{2}:\d{2})\b)");

    // Prepare for processing
    istringstream contentStream(content);
    string line;
    ptime startTime, endTime;
    bool bootStarted = false;
    int lineNum = 0;

    // Output buffer
    stringstream outputBuffer;

    while (getline(contentStream, line)) {
        lineNum++;
        auto times = findBootTimes(line, pattern);

        if (!times.first.is_not_a_date_time()) {
            if (isBootStart(line)) {
                if (bootStarted) {
                    outputBuffer << "**** Incomplete boot ****" << endl << endl;
                }
                startTime = times.first;
                bootStarted = true;
                outputBuffer << "=== Device boot ===" << endl;
                outputBuffer << lineNum << "(" << fileName << "): "
                << formatDateTime(startTime) << " Boot Start" << endl;
            }
            if (isBootComplete(line) && bootStarted) {
                endTime = times.second;
                time_duration duration = endTime - startTime;
                outputBuffer << lineNum << "(" << fileName << "): "
                << formatDateTime(endTime) << " Boot Completed" << endl;
                outputBuffer << "\tBoot Time: " << duration.total_milliseconds()
                << "ms" << endl << endl;
                bootStarted = false;
            }
        }
    }

    if (bootStarted) {
        outputBuffer << "**** Incomplete boot ****" << endl << endl;
    }

    // Write the buffered output to the file
    ofstream outputFile(fileName + ".rpt");
    if (!outputFile.is_open()) {
        cerr << "Error opening output file: " << fileName << ".rpt" << endl;
        return;
    }
    outputFile << outputBuffer.str();
    outputFile.close();
}

// Find boot start and end times from a log line
pair<ptime, ptime> findBootTimes(const string& logLine, const regex& pattern) {
    smatch matches;
    if (regex_search(logLine, matches, pattern)) {
        string dateTimeStr = matches[1].str() + " " + matches[2].str();
        ptime dateTime(time_from_string(dateTimeStr));
        return make_pair(dateTime, dateTime);
    }
    return make_pair(ptime(), ptime());
}

bool isBootComplete(const string& logLine) {
    return logLine.find("oejs.AbstractConnector:Started SelectChannelConnector@0.0.0.0:9080")
    != string::npos;
}

bool isBootStart(const string& logLine) {
    return logLine.find("(log.c.166) server started") != string::npos;
}

// Format date time as YYYY-MM-DD HH:MM:SS
string formatDateTime(const ptime& dateTime) {
    return to_iso_extended_string(dateTime.date()) + " " +
    to_simple_string(dateTime.time_of_day());
}
