#ifndef DEFAULT_DELETER_H
#define DEFAULT_DELETER_H

namespace vndn
{

/**
 * \brief a template used to delete objects
 *        by the *RefCount<> templates when the
 *        last reference to an object they manage
 *        disappears.
 *
 * \sa vndn::SimpleRefCount
 */
template <typename T>
struct DefaultDeleter {
    inline static void Delete (T *object) {
        delete object;
    }
};

} // namespace vndn

#endif /* DEFAULT_DELETER_H */
