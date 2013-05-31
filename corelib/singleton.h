/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
 */

#ifndef SINGLETON_H
#define SINGLETON_H

namespace vndn
{

/**
 * \brief a template singleton
 *
 * This template class can be used to implement the singleton pattern.
 * The underlying object will be destroyed automatically when the process
 * exits. Note that, if you call Singleton::Get again after the object has
 * been destroyed, the object will be re-created which will result in a
 * memory leak as reported by most memory leak checkers. It is up to the
 * user to ensure that Singleton::Get is never called from a static variable
 * finalizer.
 */
template <typename T>
class Singleton
{
public:
    static T *Get (void);

};

} // namespace vndn

namespace vndn
{

template <typename T>
T *
Singleton<T>::Get (void)
{
    static T object;
    return &object;
}


} // namespace vndn

#endif /* SINGLETON_H */
