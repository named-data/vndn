/*
 * Copyright (c) 2006-2007  INRIA
 * Copyright (c) 2012-2013  Davide Pesavento
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *         Davide Pesavento <davidepesa@gmail.com>
 */

#ifndef NDN_LOG_H
#define NDN_LOG_H

#include <sstream>
#include <stdint.h>

#include <yajl/yajl_gen.h>


namespace vndn
{
namespace log
{

enum LogLevel {
    NDN_LOG_NONE           = 0x00000000, // no logging

    NDN_LOG_ERROR          = 0x00000001, // serious error messages only
    NDN_LOG_LEVEL_ERROR    = 0x00000001,

    NDN_LOG_WARN           = 0x00000002, // warning messages
    NDN_LOG_LEVEL_WARN     = 0x00000003,

    NDN_LOG_NOTICE         = 0x00000004, // noteworthy messages (reserved for json output)
    NDN_LOG_LEVEL_NOTICE   = 0x00000007,

    NDN_LOG_INFO           = 0x00000008, // informational messages
    NDN_LOG_LEVEL_INFO     = 0x0000000f,

    NDN_LOG_DEBUG          = 0x00000010, // verbose debug messages
    NDN_LOG_LEVEL_DEBUG    = 0x0000001f,

    NDN_LOG_FUNCTION       = 0x00000020, // function tracing
    NDN_LOG_LEVEL_FUNCTION = 0x0000003f,

    NDN_LOG_LEVEL_ANY      = 0x00ffffff, // any logging level

    NDN_LOG_ALL            = 0x11ffffff, // print everything

    NDN_LOG_TO_CONSOLE     = 0x02000000, // print messages to console
    NDN_LOG_TO_SYSLOG      = 0x04000000, // send messages to syslog

    NDN_LOG_PREFIX_FUNC    = 0x20000000, // prefix all trace prints with function
    NDN_LOG_PREFIX_TIME    = 0x40000000, // prefix all trace prints with timestamp
    NDN_LOG_PREFIX_NODE    = 0x80000000  // prefix all trace prints with node name
};


class LogComponent
{
public:
    LogComponent(char const *name);
    bool IsEnabled(enum LogLevel level) const;
    bool IsNoneEnabled() const;
    void Enable(enum LogLevel level);
    void Disable(enum LogLevel level);
    char const *Name() const { return m_name; }
    std::string Hostname() const { return m_hostname; }

private:
    void ParseEnvVar();

    int32_t     m_levels;
    char const *m_name;
    std::string m_hostname;
};


enum JsonMsgType {
    CurrentGpsPosition      = 1,
    AppExpressedInterest    = 2,
    AppRepliedToInterest    = 3,
    InterestSatisfied       = 4,
    InterestTimedOut        = 5,
    ConfigChanged           = 6,
    LinkLayerStats          = 7,
    ContentStoreStats       = 8,
    PitStats                = 9,
    FibStats                = 10,
    PhotoReceived           = 11,
    PhotoUploaded           = 12
};

enum JsonSyntax {
    JsonNull,
    JsonArrayOpen,
    JsonArrayClose,
    JsonMapOpen,
    JsonMapClose
};

class JsonLogger
{
public:
    JsonLogger();
    ~JsonLogger();

    std::string ToString() const;

    JsonLogger &operator<<(enum JsonSyntax x);
    JsonLogger &operator<<(bool b);
    JsonLogger &operator<<(int n);
    JsonLogger &operator<<(unsigned int n);
    JsonLogger &operator<<(double d);
    JsonLogger &operator<<(const char *s);
    JsonLogger &operator<<(const std::string &s);

private:
    yajl_gen m_gen;
};


class ParameterLogger : public std::ostream
{
public:
    explicit ParameterLogger(std::ostream &os);

    template<typename T>
    ParameterLogger &operator<<(const T &param)
    {
        switch (m_itemNumber) {
        case 0: // first parameter
            m_os << param;
            break;
        default: // parameter following a previous parameter
            m_os << ", " << param;
            break;
        }
        m_itemNumber++;
        return *this;
    }

private:
    int m_itemNumber;
    std::ostream &m_os;
};


/**
 * \param name a log component name
 * \param level a logging level
 * \ingroup logging
 *
 * Enable the logging output associated with that log component.
 * The logging output can be later disabled with a call
 * to vndn::log::LogComponentDisable.
 *
 * Same as running your program with the VNDN_LOG environment
 * variable set as VNDN_LOG='name=level'
 */
void LogComponentEnable(char const *name, enum LogLevel level);

/**
 * \param level a logging level
 * \ingroup logging
 *
 * Enable the logging output for all registered log components.
 *
 * Same as running your program with the VNDN_LOG environment
 * variable set as VNDN_LOG='*=level'
 */
void LogComponentEnableAll(enum LogLevel level);

/**
 * \param name a log component name
 * \param level a logging level
 * \ingroup logging
 *
 * Disable the logging output associated with that log component.
 * The logging output can be later re-enabled with a call
 * to vndn::log::LogComponentEnable.
 */
void LogComponentDisable(char const *name, enum LogLevel level);

/**
 * \param level a logging level
 * \ingroup logging
 *
 * Disable all logging for all components.
 */
void LogComponentDisableAll(enum LogLevel level);

} // namespace log
} // namespace vndn


/**
 * \ingroup logging
 * \param name a string
 *
 * Define a Log component with a specific name. This macro
 * should be used at the top of every file in which you want
 * to use the NS_LOG macro. This macro defines a new
 * "log component" which can be later selectively enabled
 * or disabled with the vndn::log::LogComponentEnable and
 * vndn::log::LogComponentDisable functions or with the
 * VNDN_LOG environment variable.
 */
#define NS_LOG_COMPONENT_DEFINE(name) \
    static vndn::log::LogComponent g_log = vndn::log::LogComponent(name)


#ifdef NDN_LOG_ENABLE

/**
 * \ingroup debugging
 * \defgroup logging Logging
 * \brief Logging functions and macros
 *
 * LOG functionality: macros which allow developers to
 * send information out on screen. All logging messages
 * are disabled by default. To enable selected logging
 * messages, use the vndn::log::LogComponentEnable
 * function or use the VNDN_LOG environment variable
 *
 * Use the environment variable VNDN_LOG to define a ':'-separated list of
 * logging components to enable. For example (using bash syntax),
 * VNDN_LOG="OlsrAgent" would enable one component at all log levels.
 * VNDN_LOG="OlsrAgent:Ipv4L3Protocol" would enable two components,
 * at all log levels, etc.
 * VNDN_LOG="*" will enable all available log components at all levels.
 *
 * To control more selectively the log levels for each component, use
 * this syntax: VNDN_LOG='Component1=func|warn:Component2=error|debug'
 * This example would enable the 'func', and 'warn' log
 * levels for 'Component1' and the 'error' and 'debug' log levels
 * for 'Component2'.  The wildcard can be used here as well.  For example
 * VNDN_LOG='*=level_all|prefix' would enable all log levels and prefix all
 * prints with the component and function names.
 */


/**
 * \ingroup logging
 * \param level the log level
 * \param msg the message to log
 *
 * This macro allows you to log an arbitrary message at a specific
 * log level. The log message is expected to be a C++ ostream
 * message such as "my string" << aNumber << "my oth stream".
 *
 * Typical usage looks like:
 * \code
 * NS_LOG(NDN_LOG_DEBUG, "a number="<<aNumber<<", anotherNumber="<<anotherNumber);
 * \endcode
 */

#define NS_LOG(level, msg)                                          \
  do                                                                \
  {                                                                 \
      if (g_log.IsEnabled(level))                                   \
      {                                                             \
          std::ostringstream ss;                                    \
          ss << vndn::log::TimeInfo                                 \
             << vndn::log::NodeInfo;                                \
          if (g_log.IsEnabled(vndn::log::NDN_LOG_PREFIX_FUNC))      \
          {                                                         \
              ss << g_log.Name() << ":" << __FUNCTION__ << "(): ";  \
          }                                                         \
          ss << msg;                                                \
          vndn::log::LogMessage(g_log, level, ss.str());            \
      }                                                             \
  }                                                                 \
  while (false)

/**
 * \ingroup logging
 * \param msg the message to log
 *
 * Use \ref NS_LOG to output a message of level NDN_LOG_ERROR.
 */
#define NS_LOG_ERROR(msg) \
  NS_LOG(vndn::log::NDN_LOG_ERROR, msg)

/**
 * \ingroup logging
 * \param msg the message to log
 *
 * Use \ref NS_LOG to output a message of level NDN_LOG_WARN.
 */
#define NS_LOG_WARN(msg) \
  NS_LOG(vndn::log::NDN_LOG_WARN, msg)

/**
 * \ingroup logging
 * \param msg the message to log
 *
 * Use \ref NS_LOG to output a message of level NDN_LOG_NOTICE.
 */
#define NS_LOG_NOTICE(msg) \
  NS_LOG(vndn::log::NDN_LOG_NOTICE, msg)

/**
 * \ingroup logging
 * \param msg the message to log
 *
 * Use \ref NS_LOG to output a message of level NDN_LOG_INFO.
 */
#define NS_LOG_INFO(msg) \
  NS_LOG(vndn::log::NDN_LOG_INFO, msg)

/**
 * \ingroup logging
 * \param msg the message to log
 *
 * Use \ref NS_LOG to output a message of level NDN_LOG_DEBUG.
 */
#define NS_LOG_DEBUG(msg) \
  NS_LOG(vndn::log::NDN_LOG_DEBUG, msg)

/**
 * \ingroup logging
 *
 * Output the name of the function.
 */
#define NS_LOG_FUNCTION_NOARGS()                                \
  do                                                            \
  {                                                             \
      if (g_log.IsEnabled(vndn::log::NDN_LOG_FUNCTION))         \
      {                                                         \
          std::ostringstream ss;                                \
          ss << vndn::log::TimeInfo                             \
             << vndn::log::NodeInfo                             \
             << g_log.Name() << ":"                             \
             << __FUNCTION__ << "()";                           \
          vndn::log::LogMessage(g_log,                          \
                                vndn::log::NDN_LOG_FUNCTION,    \
                                ss.str());                      \
      }                                                         \
  }                                                             \
  while (false)

/**
 * \ingroup logging
 * \param parameters the parameters to output.
 *
 * If log level NDN_LOG_FUNCTION is enabled, this macro will
 * output all input parameters separated by ", ".
 *
 * Typical usage looks like:
 * \code
 * NS_LOG_FUNCTION(aNumber<<anotherNumber);
 * \endcode
 * And the output will look like:
 * \code
 * Component:Function (aNumber, anotherNumber)
 * \endcode
 */
#define NS_LOG_FUNCTION(parameters)                             \
  do                                                            \
  {                                                             \
      if (g_log.IsEnabled(vndn::log::NDN_LOG_FUNCTION))         \
      {                                                         \
          std::ostringstream ss;                                \
          ss << vndn::log::TimeInfo                             \
             << vndn::log::NodeInfo                             \
             << g_log.Name() << ":"                             \
             << __FUNCTION__ << "(";                            \
          vndn::log::ParameterLogger(ss) << parameters;         \
          ss << ")";                                            \
          vndn::log::LogMessage(g_log,                          \
                                vndn::log::NDN_LOG_FUNCTION,    \
                                ss.str());                      \
      }                                                         \
  }                                                             \
  while (false)

/**
 * \ingroup logging
 * \param type the type of the message (see JsonMsgType)
 * \param msg the message to log
 *
 * Output a JSON-formatted message of level NDN_LOG_NOTICE.
 */
#define NS_LOG_JSON(type, msg)                                  \
  do                                                            \
  {                                                             \
      if (g_log.IsEnabled(vndn::log::NDN_LOG_NOTICE))           \
      {                                                         \
          vndn::log::JsonLogger json;                           \
          json << vndn::log::JsonMapOpen                        \
               << "msgType" << type                             \
               << "msgSource" << g_log.Hostname()               \
               << msg                                           \
               << vndn::log::JsonMapClose;                      \
          vndn::log::LogMessage(g_log,                          \
                                vndn::log::NDN_LOG_NOTICE,      \
                                json.ToString());               \
      }                                                         \
  }                                                             \
  while (false)

#else /* NDN_LOG_ENABLE */

#define NS_LOG(level, msg)
#define NS_LOG_ERROR(msg)
#define NS_LOG_WARN(msg)
#define NS_LOG_NOTICE(msg)
#define NS_LOG_INFO(msg)
#define NS_LOG_DEBUG(msg)
#define NS_LOG_FUNCTION_NOARGS()
#define NS_LOG_FUNCTION(msg)
#define NS_LOG_JSON(msg)

#endif /* NDN_LOG_ENABLE */


namespace vndn
{
namespace log
{

/**
 * \ingroup logging
 *
 * Print the list of logging messages available.
 * Same as running your program with the VNDN_LOG environment
 * variable set as VNDN_LOG=print-list
 */
void PrintComponentList();

int ConvertToSyslogLevel(enum LogLevel level);

std::ostream &TimeInfo(std::ostream &os);
std::ostream &NodeInfo(std::ostream &os);

typedef void (*TimePrinter)(std::ostream &os);
typedef void (*NodePrinter)(std::ostream &os);

void SetTimePrinter(TimePrinter);
TimePrinter GetTimePrinter();

void SetNodePrinter(NodePrinter);
NodePrinter GetNodePrinter();

void LogMessage(const LogComponent &component,
                enum LogLevel level,
                const std::string &msg);

} // namespace log
} // namespace vndn

#endif /* NDN_LOG_H */
