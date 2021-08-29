//
// Created by ivan on 19-6-4.
//

// todo.
//  1. daemon thread to record to file

#ifndef __IVANS_TB_LOG__CPP14_H__
#define __IVANS_TB_LOG__CPP14_H__

#include <errno.h>
#include <string>
#include <cstring>
#include <mutex>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#define str_ std::to_string

#ifndef NDEBUG
#   define AssertLog(Expr, Msg) \
    Log::Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define AssertLog(Expr, Msg) \
    Log::Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#endif


class Log;

class Log {
  public:
    enum class Levels{
        // verbose, debug, info, warning, err, suppress
        v,d,i,w,e,s
    };

#ifdef NDEBUG
    constexpr static Levels defLevel = Levels::i;
    constexpr static Levels defRecLevel = Levels::s;

// #define LOG_DISABLE_

#else
    constexpr static Levels defLevel = Levels::v;
    constexpr static Levels defRecLevel = Levels::s;
#endif

  protected:
    Levels logLevel;
    Levels recordLevel;
    static std::mutex mu;

  private:
    static inline std::string getTimeStr();

  protected:
  public:
    explicit Log(Levels logLevel = defLevel, Levels recLevel = defRecLevel) noexcept
            : logLevel(logLevel), recordLevel(recLevel) {}

    struct GlobalLog__{
        static Log* logger(){
            static Log log;
            return &log;
        }
        static std::string & fileName(){
            static std::string fileName = "log";
            return fileName;
        }
    };

  protected:
    /*
     * basic log function
     */
    static inline void log(const std::string& tag, const std::string& content, Levels level, bool toFile);

    template <typename... Args>
    void logf(const std::string &tag, const std::string &content, Levels level, bool log2file,
              Args... args){
        // todo. pre-allocate buf.

        size_t size = snprintf( nullptr, 0, content.c_str(), args ... ) + 1; // +1 for '\0'
        char buf[size];
        snprintf( buf, size, content.c_str(), args ... );
        std:: string _content = std::string( buf, buf + size - 1 ); // exclude '\0'

        log(tag, _content, level, log2file);
    }

  public:
    template <typename... Args>
    void e(const std::string& tag, const std::string& content, Args... args){
#ifndef LOG_DISABLE_
        if (logLevel <= Levels::e)
            logf(tag, content, Levels::e, recordLevel <= Levels::e, args...);
#endif
    }

    template <typename... Args>
    void w(const std::string& tag, const std::string& content, Args... args){
#ifndef LOG_DISABLE_
        if (logLevel <= Levels::w)
            logf(tag, content, Levels::w, recordLevel <= Levels::w, args...);
#endif
    }

    template <typename... Args>
    void i(const std::string& tag, const std::string& content, Args... args){
#ifndef LOG_DISABLE_
        if (logLevel <= Levels::i)
            logf(tag, content, Levels::i, recordLevel <= Levels::i, args...);
#endif
    }

    template <typename... Args>
    void d(const std::string& tag, const std::string& content, Args... args){
#ifndef LOG_DISABLE_
        if (logLevel <= Levels::d)
            logf(tag, content, Levels::d, recordLevel <= Levels::d, args...);
#endif
    }

    template <typename... Args>
    void v(const std::string& tag, const std::string& content, Args... args){
#ifndef LOG_DISABLE_
        if (logLevel <= Levels::v)
            logf(tag, content, Levels::v, recordLevel <= Levels::v, args...);
#endif
    }

    template <typename... Args>
    void eErrno(const std::string & tag, const std::string & content, Args... args){
#ifndef LOG_DISABLE_
        if (logLevel <= Levels::e)
            logf(tag, content + "\nerrno " + str_((int)errno) + ": " + strerror(errno), "E",
                 recordLevel <= Levels::e, args...);
#endif
    }

    template <typename... Args>
    static void E(const std::string & tag, const std::string & content, Args... args){
        GlobalLog__::logger()->e(tag, content, args...);
    }

    template <typename... Args>
    static void W(const std::string & tag, const std::string & content, Args... args){
        GlobalLog__::logger()->w(tag, content, args...);
    }

    template <typename... Args>
    static void I(const std::string & tag, const std::string & content, Args... args){
        GlobalLog__::logger()->i(tag, content, args...);
    }

    template <typename... Args>
    static void D(const std::string& tag, const std::string & content, Args... args){
        GlobalLog__::logger()->d(tag, content, args...);
    }

    template <typename... Args>
    static void V(const std::string& tag, const std::string& content, Args... args){
        GlobalLog__::logger()->v(tag, content, args...);
    }

    template <typename... Args>
    static void EErrno(const std::string& tag, const std::string& content, Args... args){
        GlobalLog__::logger()->eErrno(tag, content, args...);
    }

    static void Assert(const char* expr_str, bool expr, const char* file, int line, std::string msg){
        if (!expr){
            Log::E("AssertFailure", "\nMessage:\t" + msg + "\n"
                    + "Expect:\t" + expr_str + "\n"
                    + "Where:\t" + file + ":" + str_(line));
            abort();
        }
    }

    static Log *GlobalLogger(){ return GlobalLog__::logger();}

    void setLogLevel(Levels logLevel_){ logLevel = logLevel_;}

    // record level is limited by log level
    void setRcdLevel(Levels recordLevel_){ recordLevel = recordLevel_;}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Log::getTimeStr(){
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
    system_clock::time_point now = system_clock::now();

    int ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() %1000;
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");

    return ss.str() + ":" + zeroPadNum(ms);
}

void Log::log(const std::string& tag, const std::string& content, Levels level, bool toFile){
    using namespace std;
    static std::unique_ptr<std::ofstream> logFile = nullptr;
    static mutex writeMu;

    FILE *dest;
    if (level == Levels::e){
        dest = stderr;
    }
    else{
        dest = stdout;
    }

    string pre, suf, levelStr;
    switch (level){
        case Levels::v:
            levelStr = "V";
            break;
        case Levels::d:
            levelStr = "D";
            pre = "\033[1m";
            suf = "\033[0m";
            break;
        case Levels::i:
            levelStr = "I";
            pre = "\033[32m";
            suf = "\033[0m";
            break;
        case Levels::w:
            levelStr = "W";
            pre = "\033[33m";
            suf = "\033[0m";
            break;
        case Levels::e:
            levelStr = "E";
            pre = "\033[31m";
            suf = "\033[0m";
            break;
        default:break;
    }

    string s = getTimeStr()+"|" + levelStr + "|" + tag + ":\t" + content;
    fprintf(dest, "%s%s%s\n", pre.c_str(), s.c_str() ,suf.c_str());

    if (toFile){
        lock_guard<mutex> lock(writeMu);
        if (!logFile)
            logFile = std::unique_ptr<ofstream>(new ofstream(GlobalLog__::fileName()));
        *logFile << s << "\n";
        logFile->flush();
    }
}

#undef LOG_DISABLE_

#endif //__IVANS_TB_LOG__CPP14_H__