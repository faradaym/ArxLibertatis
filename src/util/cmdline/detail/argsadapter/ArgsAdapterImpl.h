/*
 * Copyright 2013-2019 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Original source is copyright 2010 - 2011. Alexey Tsoy.
 * http://sourceforge.net/projects/interpreter11/
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef BOOST_PP_IS_ITERATING

#define DEC_N BOOST_PP_DEC(N)

#define SUPER_T args_adapter_impl<void(BOOST_PP_ENUM_PARAMS(DEC_N, A))>
#define IMPL_T \
	arg_impl<typename std::remove_cv<typename std::remove_reference<BOOST_PP_CAT(A, DEC_N)>::type>::type, DEC_N>

template <BOOST_PP_ENUM_PARAMS(N, typename A)>
struct args_adapter_impl<void(BOOST_PP_ENUM_PARAMS(N, A))>
	#if (N > 0)
	: SUPER_T
	, IMPL_T
	#endif // (N > 0)
{
	template <typename SourceType>
	explicit args_adapter_impl(SourceType & s)
		#if (N > 0)
		: SUPER_T(s)
		, IMPL_T(construct
		(s, static_cast<const typename IMPL_T::BOOST_PP_CAT(BOOST_PP_CAT(arg, DEC_N), _t) *>(0)))
		#endif // (N > 0)
	{
		ARX_UNUSED(s);
	}
};

#undef DEC_N
#undef SUPER_T

template <
	typename R
	BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, typename A)
>
struct args_adapter_impl<R(BOOST_PP_ENUM_PARAMS(N, A))>
	: args_adapter_impl<void(BOOST_PP_ENUM_PARAMS(N, A))> {
};

#endif // BOOST_PP_IS_ITERATING
