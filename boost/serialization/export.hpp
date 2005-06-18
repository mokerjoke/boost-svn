#ifndef BOOST_SERIALIZATION_EXPORT_HPP
#define BOOST_SERIALIZATION_EXPORT_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// export.hpp: set traits of classes to be serialized

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

// implementation of class export functionality.  This is an alternative to
// "forward declaration" method to provoke instantiation of derived classes
// that are to be serialized through pointers.

#include <utility>

#include <boost/config.hpp>

// if no archive headers have been included this is a no op
// this is to permit BOOST_EXPORT etc to be included in a 
// file declaration header
#if ! defined(BOOST_ARCHIVE_BASIC_ARCHIVE_HPP)
#define BOOST_CLASS_EXPORT_GUID_ARCHIVE_LIST(T, K, ASEQ)

#else
#include <boost/static_assert.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/void.hpp>
#include <boost/mpl/identity.hpp>

#include <boost/archive/detail/known_archive_types.hpp>
#include <boost/serialization/force_include.hpp>
#include <boost/serialization/extended_type_info.hpp>
#include <boost/serialization/type_info_implementation.hpp>

namespace boost {

namespace archive {
namespace detail {

// forward template declarations
class basic_pointer_iserializer;
template<class Archive, class T>
const basic_pointer_iserializer &
instantiate_pointer_iserializer(Archive * ar, T *);

class basic_pointer_oserializer;
template<class Archive, class T>
const basic_pointer_oserializer &
instantiate_pointer_oserializer(Archive * ar, T *);

namespace export_impl
{
    struct nothing{
        static void instantiate(){}
    };

    template<class Archive, class T>
    struct archive {
        struct i {
            static void invoke(){
                instantiate_pointer_iserializer(
                    static_cast<Archive *>(NULL),
                    static_cast<T *>(NULL)
                );
            }
        };
        struct o {
            static void invoke(){
                instantiate_pointer_oserializer(
                    static_cast<Archive *>(NULL),
                    static_cast<T *>(NULL)
                );
            }
        };
        static void instantiate(){
            #if defined(__GNUC__) && (__GNUC__ >= 3)
            BOOST_STATIC_ASSERT(
                Archive::is_loading::value || Archive::is_saving::value
            );
            #endif
            typedef BOOST_DEDUCED_TYPENAME mpl::eval_if<
                BOOST_DEDUCED_TYPENAME Archive::is_saving,
                mpl::identity<o>,
            // else
            BOOST_DEDUCED_TYPENAME mpl::eval_if<
                BOOST_DEDUCED_TYPENAME Archive::is_loading,
                mpl::identity<i>,
            // else
                mpl::identity<nothing>
            > >::type typex;
            typex::invoke();
        }
    };

    template<class ASeq, class T>
    struct for_each_archive {
    private:
        typedef BOOST_DEDUCED_TYPENAME mpl::pop_front<ASeq>::type tail;
        typedef BOOST_DEDUCED_TYPENAME mpl::front<ASeq>::type head;
    public:
        static void instantiate(){
            archive<head, T>::instantiate();
            typedef BOOST_DEDUCED_TYPENAME mpl::eval_if<
                mpl::empty<tail>,
                mpl::identity<nothing>,
                mpl::identity<for_each_archive<tail, T> >
            >::type typex;
            typex::instantiate();
        }
    };

} // namespace export_impl

// strictly conforming
template<class T, class ASeq>
struct export_generator {
    export_generator(){
        export_impl::for_each_archive<ASeq, T>::instantiate();
    }
    static const export_generator instance;
};

template<class T, class ASeq>
const export_generator<T, ASeq> 
    export_generator<T, ASeq>::instance;

// instantiation of this template creates a static object.
template<class T>
struct guid_initializer {
    typedef BOOST_DEDUCED_TYPENAME boost::serialization::type_info_implementation<T>::type eti_type;
    static void key_register(const char *key){
        boost::serialization::extended_type_info * eti = eti_type::get_instance();
        eti->key_register(key);
    }
    static const guid_initializer instance;
    guid_initializer(const char *key = NULL) BOOST_USED ;
};

template<class T>
guid_initializer<T>::guid_initializer(const char *key){
    if(NULL != key)
        key_register(key);
    #if BOOST_SERIALIZATION_STATIC_DATA_REGISTRATION_WORKAROUND
    boost::serialization::access::static_data_registration_workaround<T>();
    #endif
}

template<class T>
const guid_initializer<T> guid_initializer<T>::instance;

struct static_data_registration_workaround_noop_archive {
    struct is_loading { typedef mpl::bool_<false> type; BOOST_STATIC_CONSTANT(bool, value=false); };
    struct is_saving { typedef mpl::bool_<false> type; BOOST_STATIC_CONSTANT(bool, value=false); };
    template<class T> inline const void * register_type(const T * t = NULL) { return NULL; }
    template<class T> inline static_data_registration_workaround_noop_archive & operator<<(T &) { return *this; }
    template<class T> inline static_data_registration_workaround_noop_archive & operator>>(T &) { return *this; }
    template<class T> inline static_data_registration_workaround_noop_archive & operator&(T &) { return *this; }
};

} // namespace detail
} // namespace archive
} // namespace boost

// only gcc seems to be able to explicitly instantiate a static instance.
// but all can instantiate a function that refers to a static instance
namespace boost { namespace archive { namespace detail {
// note declaration to permit gcc trailing function attribute
template<class T, class ASeq>
BOOST_DLLEXPORT std::pair<const void *, const void *> 
boost_template_instantiate(T &, ASeq &) BOOST_USED;
template<class T, class ASeq>
BOOST_DLLEXPORT std::pair<const void *, const void *>
boost_template_instantiate(T &, ASeq &){
    return std::pair<const void *, const void *>(
        & export_generator<T, ASeq>::instance,
        & guid_initializer<T>::instance
    );
}
} } }

#if BOOST_SERIALIZATION_STATIC_DATA_REGISTRATION_WORKAROUND
#define BOOST_SERIALIZATION_STATIC_DATA_REGISTRATION_EXPORT_WORKAROUND(T) \
    namespace boost {                                                     \
    namespace serialization {                                             \
    namespace detail {                                                    \
    namespace {                                                           \
        void ( * export_ ## T )()                                         \
            = & static_data_registration_workaround<T>;                   \
    } } } }                                                               \
    /**/
#else
#define BOOST_SERIALIZATION_STATIC_DATA_REGISTRATION_EXPORT_WORKAROUND(T)
#endif

#define BOOST_CLASS_EXPORT_GUID_ARCHIVE_LIST(T, K, ASEQ)         \
    namespace boost {                                            \
    namespace archive {                                          \
    namespace detail {                                           \
    template<>                                                   \
    const guid_initializer<T>                                    \
        guid_initializer<T>::instance(K);                        \
    template                                                     \
    BOOST_DLLEXPORT std::pair<const void *, const void *>        \
    boost_template_instantiate(T &, ASEQ &);                     \
    } } }                                                        \
    BOOST_SERIALIZATION_STATIC_DATA_REGISTRATION_EXPORT_WORKAROUND(T)
    /**/

#endif

// check for unnecessary export.  T isn't polymorphic so there is no 
// need to export it.
#define BOOST_CLASS_EXPORT_CHECK(T)                              \
    BOOST_STATIC_WARNING(                                        \
        boost::serialization::type_info_implementation<T>        \
            ::type::is_polymorphic::value                        \
    );                                                           \
    /**/

// the default list of archives types for which code id generated
#define BOOST_CLASS_EXPORT_GUID(T, K)                            \
    BOOST_CLASS_EXPORT_GUID_ARCHIVE_LIST(                        \
        T,                                                       \
        K,                                                       \
        boost::archive::detail::known_archive_types::type \
    )                                                            \
    /**/

// the default exportable class identifier is the class name
#define BOOST_CLASS_EXPORT_ARCHIVE_LIST(T, ASEQ)   \
    BOOST_CLASS_EXPORT_GUID_ARCHIVE_LIST(T, BOOST_PP_STRINGIZE(T), A)

// the default exportable class identifier is the class name
// the default list of archives types for which code id generated
// are the originally included with this serialization system
#define BOOST_CLASS_EXPORT(T)                                    \
    BOOST_CLASS_EXPORT_GUID_ARCHIVE_LIST(                        \
        T,                                                       \
        BOOST_PP_STRINGIZE(T),                                   \
        boost::archive::detail::known_archive_types::type \
    )                                                            \
    /**/

#endif // BOOST_SERIALIZATION_EXPORT_HPP
