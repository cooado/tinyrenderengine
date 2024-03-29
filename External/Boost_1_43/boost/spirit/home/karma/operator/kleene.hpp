//  Copyright (c) 2001-2010 Joel de Guzman
//  Copyright (c) 2001-2010 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(BOOST_SPIRIT_KARMA_KLEENE_MAR_03_2007_0337AM)
#define BOOST_SPIRIT_KARMA_KLEENE_MAR_03_2007_0337AM

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/spirit/home/karma/domain.hpp>
#include <boost/spirit/home/karma/generator.hpp>
#include <boost/spirit/home/karma/meta_compiler.hpp>
#include <boost/spirit/home/karma/detail/output_iterator.hpp>
#include <boost/spirit/home/support/info.hpp>
#include <boost/spirit/home/support/unused.hpp>
#include <boost/spirit/home/support/container.hpp>
#include <boost/spirit/home/support/attributes.hpp>

#include <boost/type_traits/add_const.hpp>

namespace boost { namespace spirit
{
    ///////////////////////////////////////////////////////////////////////////
    // Enablers
    ///////////////////////////////////////////////////////////////////////////
    template <>
    struct use_operator<karma::domain, proto::tag::dereference> // enables *g
      : mpl::true_ {};

}}

///////////////////////////////////////////////////////////////////////////////
namespace boost { namespace spirit { namespace karma
{
    template <typename Subject>
    struct kleene : unary_generator<kleene<Subject> >
    {
    private:
        template <
            typename OutputIterator, typename Context, typename Delimiter
          , typename Attribute>
        bool generate_subject(OutputIterator& sink, Context& ctx
          , Delimiter const& d, Attribute const& attr) const
        {
            // Ignore return value, failing subject generators are just 
            // skipped. This allows to selectively generate items in the 
            // provided attribute.
            subject.generate(sink, ctx, d, attr);
            return true;
        }

        template <typename OutputIterator, typename Context, typename Delimiter>
        bool generate_subject(OutputIterator& sink, Context& ctx
          , Delimiter const& d, unused_type) const
        {
            // There is no way to distinguish a failed generator from a 
            // generator to be skipped. We assume the user takes responsibility
            // for ending the loop if no attribute is specified.
            return subject.generate(sink, ctx, d, unused);
        }

    public:
        typedef Subject subject_type;
        typedef typename subject_type::properties properties;

        // Build a std::vector from the subject's attribute. Note
        // that build_std_vector may return unused_type if the
        // subject's attribute is an unused_type.
        template <typename Context, typename Iterator>
        struct attribute
          : traits::build_std_vector<
                typename traits::attribute_of<Subject, Context, Iterator>::type
            >
        {};

        kleene(Subject const& subject)
          : subject(subject) {}

        template <
            typename OutputIterator, typename Context, typename Delimiter
          , typename Attribute>
        bool generate(OutputIterator& sink, Context& ctx
          , Delimiter const& d, Attribute const& attr) const
        {
            typedef typename traits::container_iterator<
                typename add_const<Attribute>::type
            >::type iterator_type;

            iterator_type it = traits::begin(attr);
            iterator_type end = traits::end(attr);

            // kleene fails only if the underlying output fails
            for (/**/; detail::sink_is_good(sink) && !traits::compare(it, end); 
                 traits::next(it))
            {
                if (!generate_subject(sink, ctx, d, traits::deref(it)))
                    break;
            }
            return detail::sink_is_good(sink);
        }

        template <typename Context>
        info what(Context& context) const
        {
            return info("kleene", subject.what(context));
        }

        Subject subject;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Generator generators: make_xxx function (objects)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Elements, typename Modifiers>
    struct make_composite<proto::tag::dereference, Elements, Modifiers>
      : make_unary_composite<Elements, kleene>
    {};

}}}

namespace boost { namespace spirit { namespace traits
{
    template <typename Subject>
    struct has_semantic_action<karma::kleene<Subject> >
      : unary_has_semantic_action<Subject> {};

}}}

#endif
