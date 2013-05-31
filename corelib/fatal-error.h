/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA, 2010 NICTA
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
 *         Quincy Tse <quincy.tse@nicta.com.au>
 *         Davide Pesavento <davidepesa@gmail.com>
 */

#ifndef NS3_FATAL_ERROR_H
#define NS3_FATAL_ERROR_H

#include <iostream>
#include <exception>
#include <cstdlib>

/**
 * \ingroup debugging
 * \brief fatal error handling
 *
 * When this macro is hit at runtime, details of filename
 * and line number is printed to stderr, and the program
 * is halted by calling std::terminate(). This will
 * trigger any clean up code registered by std::set_terminate.
 *
 * This macro is enabled unconditionally in all builds,
 * including debug and optimized builds.
 */
#define NS_FATAL_ERROR_NO_MSG()                           \
  do                                                      \
    {                                                     \
      std::cerr << "file=" << __FILE__ <<                 \
                 ", line=" << __LINE__ << std::endl;      \
      std::fflush(0);                                     \
      std::cout.flush();                                  \
      std::cerr.flush();                                  \
      std::clog.flush();                                  \
      std::terminate();                                   \
    }                                                     \
  while (false)

/**
 * \ingroup debugging
 * \brief fatal error handling
 *
 * \param msg message to output when this macro is hit.
 *
 * When this macro is hit at runtime, the user-specified
 * error message is printed to stderr, followed by a call
 * to the NS_FATAL_ERROR_NO_MSG() macro which prints the
 * details of filename and line number to stderr. The
 * program will be halted by calling std::terminate(),
 * triggering any clean up code registered by
 * std::set_terminate.
 *
 * This macro is enabled unconditionally in all builds,
 * including debug and optimized builds.
 */
#define NS_FATAL_ERROR(msg)                             \
  do                                                    \
    {                                                   \
      std::cerr << "msg=\"" << msg << "\", ";           \
    }                                                   \
  while (false)

#endif /* FATAL_ERROR_H */
