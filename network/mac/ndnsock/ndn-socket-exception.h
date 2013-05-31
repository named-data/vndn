/*
 * Copyright (c) 2012-2013 Giulio Grassi <giulio.grassi86@gmail.com>
 *                         Davide Pesavento <davidepesa@gmail.com>
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
 */

#ifndef NDNSOCKETEXCEPTION_H_
#define NDNSOCKETEXCEPTION_H_

#include <exception>
#include <string>

namespace vndn
{

class NdnSocketException : public std::exception
{
public:
    /**
     * \brief Create an NdnSocketException with a specified error message
     *
     * \param strError error message that has to be reported by the exception
     * */
    NdnSocketException(std::string err) : errorDesc(err) {}

    virtual ~NdnSocketException() throw() {}

    /**
     * \brief Report the error message specified by the constructor on the exception
     * */
    virtual const char *what() const throw() { return errorDesc.c_str(); }

protected:
    /**
     * \brief Error message
     * */
    std::string errorDesc;
};

} /* namespace vndn */

#endif /* NDNSOCKETEXCEPTION_H_ */
