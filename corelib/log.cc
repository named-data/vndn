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

#include "log.h"
#include "fatal-error.h"
#include "pretty-print.h"

#include <errno.h>
#include <limits.h>
#include <list>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

using std::string;


namespace vndn
{
namespace log
{

TimePrinter g_logTimePrinter = 0;
NodePrinter g_logNodePrinter = 0;

typedef std::list<std::pair <std::string, LogComponent *> > ComponentList;
typedef std::list<std::pair <std::string, LogComponent *> >::iterator ComponentListI;

static ComponentList *GetComponentList()
{
    static ComponentList components;
    return &components;
}

static class PrintList
{
public:
    PrintList();
} g_printList;

PrintList::PrintList()
{
    char *envVar = ::getenv("VNDN_LOG");
    if (!envVar)
        return;

    string env = envVar;
    string::size_type cur = 0;
    string::size_type next = 0;

    while (next != string::npos) {
        next = env.find_first_of (":", cur);
        string tmp = string (env, cur, next - cur);
        if (tmp == "print-list") {
            PrintComponentList();
            exit(0);
            break;
        }
        cur = next + 1;
    }
}


LogComponent::LogComponent(char const *name)
    : m_levels(0)
    , m_name(name)
{
    ParseEnvVar();

    ComponentList *components = GetComponentList();
    for (ComponentListI i = components->begin(); i != components->end(); ++i) {
        if (i->first == name) {
            NS_FATAL_ERROR("Log component \"" << name << "\" has already been registered once.");
        }
    }
    components->push_back(std::make_pair(name, this));

    // enable error logging to console by default
    Enable(static_cast<LogLevel>(NDN_LOG_LEVEL_ERROR |
                                 NDN_LOG_TO_CONSOLE |
                                 NDN_LOG_PREFIX_FUNC));

    if (m_hostname.empty()) {
        char buf[HOST_NAME_MAX + 1];
        if (::gethostname(buf, sizeof(buf)) == 0) {
            m_hostname = string(buf);
        } else {
            int e = errno;
            std::cerr << "gethostname() failed: " << strerror(e) << std::endl;
        }
    }
}

void LogComponent::ParseEnvVar()
{
    char *envVar = ::getenv("VNDN_LOG");
    if (!envVar)
        return;

    string env = envVar;
    string::size_type cur = 0;
    string::size_type next = 0;

    while (next != string::npos) {
        next = env.find_first_of (":", cur);
        string tmp = string (env, cur, next - cur);
        string::size_type equal = tmp.find ("=");
        string component;
        if (equal == string::npos) {
            component = tmp;
            if (component == m_name || component == "*") {
                // enable default logging levels
                Enable(static_cast<LogLevel>
                       (NDN_LOG_LEVEL_DEBUG |
                        NDN_LOG_TO_CONSOLE |
                        NDN_LOG_PREFIX_FUNC));
                return;
            }
        } else {
            component = tmp.substr (0, equal);
            if (component == m_name || component == "*") {
                int level = 0;
                string::size_type cur_lev;
                string::size_type next_lev = equal;
                do {
                    cur_lev = next_lev + 1;
                    next_lev = tmp.find ("|", cur_lev);
                    string lev = tmp.substr (cur_lev, next_lev - cur_lev);
                    if (lev == "error") {
                        level |= NDN_LOG_ERROR;
                    } else if (lev == "warn") {
                        level |= NDN_LOG_WARN;
                    } else if (lev == "notice") {
                        level |= NDN_LOG_NOTICE;
                    } else if (lev == "info") {
                        level |= NDN_LOG_INFO;
                    } else if (lev == "debug") {
                        level |= NDN_LOG_DEBUG;
                    } else if (lev == "function") {
                        level |= NDN_LOG_FUNCTION;
                    } else if (lev == "all") {
                        level |= NDN_LOG_ALL;
                    } else if (lev == "prefix_func") {
                        level |= NDN_LOG_PREFIX_FUNC;
                    } else if (lev == "prefix_time") {
                        level |= NDN_LOG_PREFIX_TIME;
                    } else if (lev == "prefix_node") {
                        level |= NDN_LOG_PREFIX_NODE;
                    } else if (lev == "level_error") {
                        level |= NDN_LOG_LEVEL_ERROR;
                    } else if (lev == "level_warn") {
                        level |= NDN_LOG_LEVEL_WARN;
                    } else if (lev == "level_notice") {
                        level |= NDN_LOG_LEVEL_NOTICE;
                    } else if (lev == "level_info") {
                        level |= NDN_LOG_LEVEL_INFO;
                    } else if (lev == "level_debug") {
                        level |= NDN_LOG_LEVEL_DEBUG;
                    } else if (lev == "level_function") {
                        level |= NDN_LOG_LEVEL_FUNCTION;
                    } else if (lev == "console") {
                        level |= NDN_LOG_TO_CONSOLE;
                    } else if (lev == "syslog") {
                        level |= NDN_LOG_TO_SYSLOG;
                    }
                } while (next_lev != string::npos);

                Enable(static_cast<LogLevel>(level));
            }
        }
        cur = next + 1;
    }
}

bool LogComponent::IsEnabled(LogLevel level) const
{
    return (m_levels & level) ? true : false;
}

bool LogComponent::IsNoneEnabled() const
{
    return !IsEnabled(NDN_LOG_LEVEL_ANY);
}

void LogComponent::Enable(LogLevel level)
{
    m_levels |= level;
}

void LogComponent::Disable(LogLevel level)
{
    m_levels &= ~level;
}


JsonLogger::JsonLogger()
{
    m_gen = yajl_gen_alloc(NULL);
}

JsonLogger::~JsonLogger()
{
    yajl_gen_free(m_gen);
}

std::string JsonLogger::ToString() const
{
    const unsigned char *buf;
    size_t len;

    if (yajl_gen_get_buf(m_gen, &buf, &len) == yajl_gen_status_ok) {
        string s(reinterpret_cast<const char *>(buf), len);
        yajl_gen_clear(m_gen);
        return s;
    } else {
        std::cerr << "yajl_gen_get_buf() failed" << std::endl;
        return string();
    }
}

JsonLogger &JsonLogger::operator<<(enum JsonSyntax x)
{
    switch (x) {
    case JsonNull:
        yajl_gen_null(m_gen);
        break;
    case JsonArrayOpen:
        yajl_gen_array_open(m_gen);
        break;
    case JsonArrayClose:
        yajl_gen_array_close(m_gen);
        break;
    case JsonMapOpen:
        yajl_gen_map_open(m_gen);
        break;
    case JsonMapClose:
        yajl_gen_map_close(m_gen);
        break;
    }

    return *this;
}

JsonLogger &JsonLogger::operator<<(bool b)
{
    yajl_gen_bool(m_gen, b);
    return *this;
}

JsonLogger &JsonLogger::operator<<(int n)
{
    yajl_gen_integer(m_gen, n);
    return *this;
}

JsonLogger &JsonLogger::operator<<(unsigned int n)
{
    yajl_gen_integer(m_gen, n);
    return *this;
}

JsonLogger &JsonLogger::operator<<(double d)
{
    yajl_gen_double(m_gen, d);
    return *this;
}

JsonLogger &JsonLogger::operator<<(const char *s)
{
    yajl_gen_string(m_gen, reinterpret_cast<const unsigned char *>(s), strlen(s));
    return *this;
}

JsonLogger &JsonLogger::operator<<(const string &s)
{
    yajl_gen_string(m_gen, reinterpret_cast<const unsigned char *>(s.data()), s.length());
    return *this;
}


ParameterLogger::ParameterLogger(std::ostream &os)
    : m_itemNumber(0)
    , m_os(os)
{
}


void LogComponentEnable(const char *name, LogLevel level)
{
    ComponentList *components = GetComponentList();
    ComponentListI i;
    for (i = components->begin(); i != components->end(); ++i) {
        if (i->first.compare(name) == 0) {
            i->second->Enable(level);
            return;
        }
    }
}

void LogComponentEnableAll(LogLevel level)
{
    ComponentList *components = GetComponentList();
    for (ComponentListI i = components->begin(); i != components->end(); ++i) {
        i->second->Enable(level);
    }
}

void LogComponentDisable(const char *name, LogLevel level)
{
    ComponentList *components = GetComponentList();
    for (ComponentListI i = components->begin(); i != components->end(); ++i) {
        if (i->first.compare(name) == 0) {
            i->second->Disable(level);
            break;
        }
    }
}

void LogComponentDisableAll(LogLevel level)
{
    ComponentList *components = GetComponentList();
    for (ComponentListI i = components->begin(); i != components->end(); ++i) {
        i->second->Disable(level);
    }
}

void PrintComponentList()
{
    ComponentList *components = GetComponentList();
    for (ComponentListI i = components->begin(); i != components->end(); i++) {
        std::list<string> enabled;

        if (i->second->IsEnabled(NDN_LOG_TO_CONSOLE)) {
            enabled.push_back("console");
        }
        if (i->second->IsEnabled(NDN_LOG_TO_SYSLOG)) {
            enabled.push_back("syslog");
        }

        if (i->second->IsNoneEnabled()) {
            enabled.push_back("none");
        }
        if (i->second->IsEnabled(NDN_LOG_ERROR)) {
            enabled.push_back("error");
        }
        if (i->second->IsEnabled(NDN_LOG_WARN)) {
            enabled.push_back("warn");
        }
        if (i->second->IsEnabled(NDN_LOG_NOTICE)) {
            enabled.push_back("notice");
        }
        if (i->second->IsEnabled(NDN_LOG_INFO)) {
            enabled.push_back("info");
        }
        if (i->second->IsEnabled(NDN_LOG_DEBUG)) {
            enabled.push_back("debug");
        }
        if (i->second->IsEnabled(NDN_LOG_FUNCTION)) {
            enabled.push_back("function");
        }

        std::cout << i->first << "=" << enabled << std::endl;
    }
}

int ConvertToSyslogLevel(LogLevel level)
{
    switch (level) {
    case NDN_LOG_ERROR:     return LOG_ERR;
    case NDN_LOG_WARN:      return LOG_WARNING;
    case NDN_LOG_NOTICE:    return LOG_NOTICE;
    case NDN_LOG_INFO:      return LOG_INFO;
    case NDN_LOG_DEBUG:     return LOG_DEBUG;
    case NDN_LOG_FUNCTION:  return LOG_DEBUG;
    default:                return LOG_DEBUG;
    }
}

std::ostream &TimeInfo(std::ostream &os)
{
    if (false) { // LogComponent::IsEnabled(NDN_LOG_PREFIX_TIME)
        TimePrinter printer = GetTimePrinter();
        if (printer) {
            (*printer)(os);
            os << " ";
        }
    }
    return os;
}

std::ostream &NodeInfo(std::ostream &os)
{
    if (false) { // LogComponent::IsEnabled(NDN_LOG_PREFIX_NODE)
        NodePrinter printer = GetNodePrinter();
        if (printer) {
            (*printer)(os);
            os << " ";
        }
    }
    return os;
}

void SetTimePrinter(TimePrinter printer)
{
    g_logTimePrinter = printer;
}

TimePrinter GetTimePrinter()
{
    return g_logTimePrinter;
}

void SetNodePrinter(NodePrinter printer)
{
    g_logNodePrinter = printer;
}

NodePrinter GetNodePrinter()
{
    return g_logNodePrinter;
}

void LogMessage(const LogComponent &component, LogLevel level, const std::string &msg)
{
    if (component.IsEnabled(NDN_LOG_TO_CONSOLE)) {
        std::clog << msg << std::endl;
    }

    if (component.IsEnabled(NDN_LOG_TO_SYSLOG)) {
        ::syslog(ConvertToSyslogLevel(level), "%s", msg.c_str());
    }
}

} // namespace log
} // namespace vndn
