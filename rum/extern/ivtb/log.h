/**
 * @brief
 *
 * Created by ivan on 2021/7/14.
 * @copyright Copyright (c) ivan 2021
 */
#ifndef IVTB_LOG_LOG_H_
#define IVTB_LOG_LOG_H_

#include <unistd.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <mutex>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <atomic>

/*
 * log level control macro:
 * LOG_SUPRESS_D
 * LOG_SUPRESS_I
 * LOG_SUPRESS_W
 * LOG_SUPRESS_E
 * LOG_SUPRESS_S
*/

/* logging macros */

#define AssertLog(Expr, Msg) \
if(!(Expr)) ivtb::Log::AssertFailure(#Expr, __FILE__, __LINE__, Msg)


namespace ivtb{

class Log;

class Log {
  public:
    enum class Level{
        // verbose, debug, info, warning, err, suppress
        v,d,i,w,e,s
    };

    enum class Destination{
        Std = 0, File = 1,
    };

#ifdef NDEBUG
    constexpr static Level kDefStdLevel = Level::i;
    constexpr static Level kDefFileLevel = Level::s;
#else
    constexpr static Level kDefStdLevel = Level::v;
    constexpr static Level kDefFileLevel = Level::s;
#endif

    static constexpr size_t kMaxFileSize = 100e3;

  protected:
    Level min_std_level_;
    Level min_file_level_;
    std::unique_ptr<std::ofstream> file_ = nullptr;
    std::streampos file_start_;
    std::mutex file_mu_;
    std::string file_path_;
    std::string path_prefix_ = "./log";
    unsigned int part_n_ = 0;
    bool auto_suffix_ = true;
    int logger_n_;

  private:
    static inline std::string getTimeStr(const std::chrono::system_clock::time_point &t);

  protected:
  public:
    explicit Log(Level std_level = kDefStdLevel, Level file_level = kDefFileLevel) noexcept
            : min_std_level_(std_level), min_file_level_(file_level) {
        static std::atomic<int> logger_pool {0};

        logger_n_ = logger_pool.fetch_add(1, std::memory_order_relaxed);
        path_prefix_ = "./logger" + std::to_string(logger_n_) + "_";
        genFilePath();
    }

  protected:
    void genFilePath(){
        using namespace std::chrono;

        if (auto_suffix_){
            system_clock::time_point now = system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&in_time_t), "_%Y-%m-%d_%H-%M-%S_") << getpid();
            file_path_ = path_prefix_ + ss.str();
        }
        else{
            file_path_ = path_prefix_;
        }
    }

    /*
     * basic log function that takes content as string
     */
    inline void logString(const std::string& tag, const std::string& content, Level level);

    /*
     * basic log function that takes printf like format args
     */
    template <typename... Args>
    void logFormat(const std::string &tag, const std::string &content, Level level, Args... args){
        if (level < min_std_level_ && level < min_file_level_) return;

        // pre-allocate buf could help, but that makes multi threading logging ugly
        size_t size = snprintf( nullptr, 0, content.c_str(), args ... ) + 1; // +1 for '\0'
        char buf[size];
        snprintf( buf, size, content.c_str(), args ... );
        std:: string log_str = std::string(buf, buf + size - 1 ); // exclude '\0'

        logString(tag, log_str, level);
    }

  public:

    void setLogLevel(Destination destination, Level level){
        switch (destination){
            case Destination::Std:
                min_std_level_ = level;
                return;
            case Destination::File:
                min_file_level_ = level;
                return;
        }
    }

    void setFilePrefix(const std::string &prefix){
        path_prefix_ = prefix;
        genFilePath();
    }

    void setFileAutoSuffix(bool enabled){
        auto_suffix_ = enabled;
        genFilePath();
    }

    template <typename... Args>
    void e(const std::string& tag, const std::string& content, Args... args){
#if !defined(LOG_SUPRESS_S)
        logFormat(tag, content, Level::e, args...);
#endif
    }

    template <typename... Args>
    void w(const std::string& tag, const std::string& content, Args... args){
#if !defined(LOG_SUPRESS_S) && !defined(LOG_SUPRESS_E)
        logFormat(tag, content, Level::w, args...);
#endif
    }

    template <typename... Args>
    void i(const std::string& tag, const std::string& content, Args... args){
#if !defined(LOG_SUPRESS_S) && !defined(LOG_SUPRESS_E) && !defined(LOG_SUPRESS_W)
        logFormat(tag, content, Level::i, args...);
#endif
    }

    template <typename... Args>
    void d(const std::string& tag, const std::string& content, Args... args){
#if !defined(LOG_SUPRESS_S) && !defined(LOG_SUPRESS_E) && !defined(LOG_SUPRESS_W) && !defined(LOG_SUPRESS_I)
        logFormat(tag, content, Level::d, args...);
#endif
    }

    template <typename... Args>
    void v(const std::string& tag, const std::string& content, Args... args){
#if !defined(LOG_SUPRESS_S) && !defined(LOG_SUPRESS_E) && !defined(LOG_SUPRESS_W) && !defined(LOG_SUPRESS_I) && !defined(LOG_SUPRESS_D)
        logFormat(tag, content, Level::v, args...);
#endif
    }

    template <typename... Args>
    void eErrno(const std::string & tag, const std::string & content, Args... args){
#ifndef LOG_SUPRESS_S
        logFormat(tag,
                  content + "\nerrno " + std::to_string((int) errno) + ": " + strerror(errno),
                  Level::e,
                  args...);
#endif
    }

    /************************************
     * global logging functions
     ************************************/

    static Log *GlobalLogger(){
        static Log log;
        return &log;
    }

    template <typename... Args>
    static void E(const std::string & tag, const std::string & content, Args... args){
        GlobalLogger()->e(tag, content, args...);
    }

    template <typename... Args>
    static void W(const std::string & tag, const std::string & content, Args... args){
        GlobalLogger()->w(tag, content, args...);
    }

    template <typename... Args>
    static void I(const std::string & tag, const std::string & content, Args... args){
        GlobalLogger()->i(tag, content, args...);
    }

    template <typename... Args>
    static void D(const std::string& tag, const std::string & content, Args... args){
        GlobalLogger()->d(tag, content, args...);
    }

    template <typename... Args>
    static void V(const std::string& tag, const std::string& content, Args... args){
        GlobalLogger()->v(tag, content, args...);
    }

    template <typename... Args>
    static void EErrno(const std::string& tag, const std::string& content, Args... args){
        GlobalLogger()->eErrno(tag, content, args...);
    }

    static void AssertFailure(const char* expr_str, const char* file, int line, std::string msg){
        GlobalLogger()->logString("AssertFailure", "\n"
                "Message:\t" + msg + "\n"
                "Expect:\t" + expr_str + "\n"
                "Where:\t" + file + ":" + std::to_string(line),
                Level::e);
        abort();
    }

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Log::getTimeStr(const std::chrono::system_clock::time_point &t){
    using namespace std;
    auto zeroPadNum = [](int num){
        constexpr int LEN =3;
        stringstream ss;

        // the number is converted to string with the help of stringstream
        ss << num;
        string ret;
        ss >> ret;

        // Append zero chars
        int str_length = ret.length();
        for (int i = 0; i < LEN - str_length; i++)
            ret = "0" + ret;
        return ret;
    };

    using namespace std::chrono;
    typedef duration<int, ratio_multiply<hours::period, ratio<24> >::type> days;

    int ms = std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()).count() %1000;
    auto in_time_t = std::chrono::system_clock::to_time_t(t);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");

    return ss.str() + ":" + zeroPadNum(ms);
}

void Log::logString(const std::string& tag, const std::string& content, Level level){
    using namespace std;

    FILE *dest;
    if (level == Level::e){
        dest = stderr;
    }
    else{
        dest = stdout;
    }

    string pre, suf, level_str;
    switch (level){
        case Level::v:
            level_str = "V";
            break;
        case Level::d:
            level_str = "D";
            pre = "\033[1m";
            suf = "\033[0m";
            break;
        case Level::i:
            level_str = "I";
            pre = "\033[32m";
            suf = "\033[0m";
            break;
        case Level::w:
            level_str = "W";
            pre = "\033[33m";
            suf = "\033[0m";
            break;
        case Level::s:
        case Level::e:
            level_str = "E";
            pre = "\033[31m";
            suf = "\033[0m";
            break;
        default:break;
    }

    auto t = std::chrono::system_clock::now();
    string s = getTimeStr(t)+"|" + level_str + "|" + tag + ":\t" + content;

    if (level>=min_std_level_)
        fprintf(dest, "%s%s%s\n", pre.c_str(), s.c_str() ,suf.c_str());

    if (level>=min_file_level_){
        lock_guard<mutex> lock(file_mu_);

        bool create_file = true;
        if (file_ && file_->tellp() - file_start_ < kMaxFileSize)
            create_file = false;

        if (create_file){
            string part_str = part_n_==0? "" : ".part" + std::to_string(part_n_);
            file_ = std::unique_ptr<ofstream>(new ofstream(file_path_ + part_str +".txt"));
            // todo ivan. test file creation failure.
            file_start_ = file_->tellp();
            ++part_n_;
        }

        *file_ << s << "\n";
        file_->flush();
    }

}

}

#endif //IVTB_LOG_LOG_H_
