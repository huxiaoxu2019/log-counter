#include <iostream>
#include <thread>
#include <mutex>
#include <deque>
#include <fstream>
#include <limits>
#include <chrono>
#include <unordered_map>

using namespace std;

// 从日志文件的第几行开始处理
#define START_LINE_NUM_OF_FILE  1
// 统计日志文件路径
#define FILENAME "/Users/huxiaoxu/Desktop/1.mp4"
// 状态信息文件路径
#define STAT_FILENAME "/Users/huxiaoxu/Desktop/stat"
// 执行周期（随眠时间）
#define POP_INTERVAL 1
// 每个 POP_INTERVAL 期间 pop 的最大日志数
#define POP_COUNT 10
#define PUSH_INTERVAL 1
#define PUSH_COUNT 10
// 60s 统计60s内有请求的人数作为在线人数
#define TIME_RANGE 5

struct Log {
    string ip;
    int time;
    Log(string i, int t) : ip(i), time(t) {};
};

deque<Log> dq;
unordered_map<string, int> hm;
mutex mtx;
int cur_line_read_file = START_LINE_NUM_OF_FILE;

fstream& go_to_line(fstream& file, unsigned int num) {
    file.seekg(ios::beg);
    for (int i = 0; i < num - 1; ++i) {
        file.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return file;
}

void push_log() {
    fstream file(FILENAME, fstream::in);
    go_to_line(file, START_LINE_NUM_OF_FILE);
    string line;
    int i = 0, lastp = file.tellp();
    while (true) {
        {
            lock_guard<mutex> lck(mtx);
            i = 0;
            while (i++ < PUSH_COUNT) {
                file = fstream(FILENAME, fstream::in);
                file.seekp(lastp);
                file >> line;
                if (file.tellp() == -1) break;
                lastp = file.tellp();
                if (line.empty()) continue;
                ++cur_line_read_file;
                ++hm[line];
                dq.push_front(Log(line, time(NULL)));
            }
        }
        this_thread::sleep_for(chrono::milliseconds (PUSH_INTERVAL));
    }
}

void pop_log() {
    int i;
    fstream file(STAT_FILENAME, fstream::out);
    while (true) {
        {
            lock_guard<mutex> lck(mtx);
            i = 0;
            while (!dq.empty() && i++ < POP_COUNT) {
                auto log = dq.back();
                if (time(NULL) - log.time < TIME_RANGE) {
                    break;
                } 
                dq.pop_back();
                --hm[log.ip];
                if (hm[log.ip] <= 0) {
                    hm.erase(log.ip);
                }
            }
            string buffer;
            buffer = "number of unique logs: " + to_string(hm.size()) + "\nnumber of logs: " + to_string(dq.size()) + "\nread file line:" + to_string(cur_line_read_file);
            file.seekg(0);
            file.write(buffer.c_str(), buffer.size());
        }

        this_thread::sleep_for(chrono::milliseconds (POP_INTERVAL));
    }
}

int main() {
    thread push_t(push_log);
    thread pop_t(pop_log);
    push_t.join();
    pop_t.join(); 
}
