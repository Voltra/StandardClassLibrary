#pragma once

#include <scl/macros.h>
#include <scl/exceptions/EmptyOptionalAccess.h>
#include <scl/tools/meta/enable_if.h>
#include <scl/tools/meta/void_t.h>
#include <scl/tools/meta/is_same.h>
#include <scl/tools/meta/is_instance.h>
#include <scl/tools/meta/exists.h>
#include <scl/tools/meta/type_check.h>
#include <scl/tools/meta/type_query.h>
#include <scl/utils/toString.h>
#include <iostream>

#include <scl/tools/iostream/nl.h>
#include <scl/tools/meta/conditional.h>

using scl::tools::iostream::nl;

//TODO: Fix copy/move problems

namespace scl{
	namespace utils{
		/**
		 * An empty class serving as the type of an empty Optional<T>
		 */
		struct None final{
			bool operator==(None) const{ return true; }
			bool operator!=(None) const{ return false; }
			bool operator<(None) const{ return false; }
			bool operator<=(None) const{ return true; }
			bool operator>(None) const{ return false; }
			bool operator>=(None) const{ return true; }
		};

		/**
		 * @var none
		 * A constant global variable of type None
		 */
		const None none = None{};

		/**
		 * @class ToString<None>
		 * A specialization of ToString for None type
		 */
		template <>
		struct ToString<None>{
			/**
			 * String representation of a None object
			 * @return String representation of a None object
			 */
			constexpr stringLiteral operator()(const None&) const{ return "[none ; scl::utils::None]"; }
		};

		/**
		 * Base class for optionals
		 * @tparam T being the optional'd type
		 */
		template <class T>
		class base_optional{
			protected:
				/**
				 * @var valueFlag
				 * determines whether or not there's a value inside this Optional<T>
				 */
				bool valueFlag = false;

				/**
				 * @var payload
				 * A scl::tools::meta::aligned_storage_t of the correct size and alignment
				 * to maintain an instance of the object
				 */
				META::aligned_storage_t<sizeof(T), alignof(T)> payload = {};

			public:
				base_optional() : valueFlag{false}, payload{} {
				}

				/**
				 * Determines whether or not this Optional<T> is empty
				 * @return TRUE if there's a value, FALSE otherwise
				 */
				bool hasValue() const{ return this->valueFlag; }

				/**
				 * Retrieves the value stored in this Optional<T>
				 * @return a const& to the value stored
				 * @throws scl::exceptions::EmptyOptionalAccess if there's no value
				 */
				const T& get() const{
					if(!this->hasValue())
						throw exceptions::EmptyOptionalAccess{};

					return *reinterpret_cast<const T*>(&payload);
				}

				/**
				 * Mutable accessor for the value stored
				 * @return a reference to the value stored
				 * @throws scl::exceptions::EmptyOptionalAccess if there's no value
				 */
				T& get(){
					if(!this->hasValue())
						throw exceptions::EmptyOptionalAccess{};

					//reinterpret_cast is allowed here since we allocated for a T
					return *reinterpret_cast<T*>(&payload);
				}
		};

		/**
		 * Base class for optionals that accept copyable types
		 * @tparam T
		 */
		template <class T>
		struct base_copyable_optional : public virtual base_optional<T>{
			using base_optional<T>::base_optional;
			using base_optional<T>::get;
			using base_optional<T>::hasValue;

			/**
			 * Creates a non empty optional with the given value (copy)
			 * @param value being the value to assign from
			 */
			base_copyable_optional(const T& value){
				this->valueFlag = true;
//					this->payload.value = value;
				new(&this->payload)T{value};
			}

			base_copyable_optional() : base_optional<T>(){}

			/**
			 * Copy constructor
			 */
			base_copyable_optional(const base_copyable_optional& o){
				this->valueFlag = o.valueFlag;
				if(o.hasValue())
//						this->payload.value = o.payload.value;
					new(&(this->payload))T{o.get()};
			};

			/**
			 * Copy assignment operator
			 * @return a reference to this Optional<T>
			 */
			base_copyable_optional& operator=(const base_copyable_optional& o){
				this->valueFlag = o.valueFlag;
				if(o.valueFlag)
//						this->payload.value = o.payload.value;
					new(&(this->payload))T{o.get()};

				return *this;
			};

			/**
			 * Automatic conversion operator
			 * @return a copy of the underlying value
			 * @throws scl::exceptions::EmptyOptionalAccess if there's no value
			 */
			explicit operator T() const{ return this->get(); }
		};

		/**
		 * Base class for optionals that accept non copyable types
		 * @tparam T
		 */
		template <class T>
		struct base_noncopyable_optional : public virtual base_optional<T>{
			using base_optional<T>::base_optional;
			using base_optional<T>::get;
			using base_optional<T>::hasValue;

			base_noncopyable_optional() : base_optional<T>(){}

			base_noncopyable_optional(const base_noncopyable_optional&) = delete;
			base_noncopyable_optional& operator=(const base_noncopyable_optional&) = delete;
			operator T() const = delete;
		};

		/**
		 * Base class for optionals that accept movable types
		 * @tparam T
		 */
		template <class T>
		struct base_movable_optional : public virtual base_optional<T>{
			using base_optional<T>::base_optional;
			using base_optional<T>::get;
			using base_optional<T>::hasValue;

			/**
			 * Creates a non empty optional with the given value (move)
			 * @param value being the value to assign from
			 */
			base_movable_optional(T&& value){
				this->valueFlag = true;
//					this->payload.value = std::move(value);
				new(&(this->payload))T{std::move(value)};
			}

			base_movable_optional() : base_optional<T>(){}

			/**
			 * Move constructor
			 */
			base_movable_optional(base_movable_optional&& o){
				this->valueFlag = o.valueFlag;
				if(o.hasValue())
//						this->payload.value = std::move(o.payload.value);
					this->payload = std::move(o.payload);
			};

			/**
			 * Move assignment operator
			 * @return a reference to this Optional<T>
			 */
			base_movable_optional& operator=(base_movable_optional&& o) noexcept{
				this->valueFlag = o.valueFlag;
				if(o.valueFlag)
//						this->payload.value = std::move(o.payload.value);
					this->payload = std::move(o.payload);

				return *this;
			};
		};

		/**
		 * Base class for optionals that accept non movable types
		 * @tparam T
		 */
		template <class T>
		struct base_nonmovable_optional : public virtual base_optional<T>{
			using base_optional<T>::base_optional;
			using base_optional<T>::get;
			using base_optional<T>::hasValue;

			base_nonmovable_optional() : base_optional<T>(){}

			base_nonmovable_optional(base_nonmovable_optional&&) = delete;
			base_nonmovable_optional& operator=(base_nonmovable_optional&&) = delete;
		};

		template <class T>
		using copy_optional_base = META::conditional_t<META::is_copyable<T>(), base_copyable_optional<T>, base_noncopyable_optional<T>>;

		template <class T>
		using move_optional_base = META::conditional_t<META::is_movable<T>(), base_movable_optional<T>, base_nonmovable_optional<T>>;

		/**
		 * A class that allows the use of optional types (might be there)
		 * @tparam T being the value type that is optional
		 * @warning Optional defines move and/or copy semantics only if the
		 * underlying type does as well
		 */
		template <class T>
		class Optional : public copy_optional_base<T>/*, public move_optional_base<T>*/{
			public:
				/*using move_optional_base<T>::move_optional_base;
				using move_optional_base<T>::operator=;*/
				using copy_optional_base<T>::copy_optional_base;
				using copy_optional_base<T>::operator=;

				using copy_optional_base<T>::get;
				using copy_optional_base<T>::hasValue;

			protected:
//				static constexpr bool is_movable = META::is_same<move_optional_base<T>, base_movable_optional<T>>();
//					return META::is_movable<T>();

				static constexpr bool is_copyable =  META::is_same<copy_optional_base<T>, base_copyable_optional<T>>();
//					return META::is_copyable<T>();

			public:
				/**
				 * Default constructor, no value and empty payload
				 */
				Optional() : /*move_optional_base<T>(),*/ copy_optional_base<T>(){
				}

				/**
				 * Instantiate an optional via an instance of None
				 */
				Optional(None) : Optional(){
				}

#define TPL template <class U, class = META::enable_if_t<\
	!META::is_same<META::decay_t<U>, META::decay_t<T>>()\
	&& !META::is_same<META::decay_t<U>, None>()\
	&& !META::is_same<META::decay_t<U>, Optional<T>>()\
>>
				/**
				 * Implicit conversion copy constructor
				 * @tparam U being the type to implicitly convert from
				 * @param value being the value to construct from
				 */
				TPL
				Optional(const U& value){
					this->valueFlag = true;
//					this->payload.value = value;
					new(&this->payload)T{value};
				}

				/**
				 * Implicit conversion move constructor
				 * @tparam U being the type to implicitly convert from
				 * @param value being the value to construct from
				 */
				TPL
				Optional(U&& value){
					this->valueFlag = true;
//					this->payload.value = std::move(value);
					new(&this->payload)T{value};
				}
#undef TPL

				/**
				 * A semantic alias for Optional<T>::get
				 * @return a const& to the stored value
				 * @throws scl::exceptions::EmptyOptionalAccess if there's no value
				 */
				const T& value() const{ return this->get(); }

				/**
				 * A semantic alias for Optional<T>::get
				 * @return a const& to the value stored
				 * @throws scl::exceptions::EmptyOptionalAccess if there's no value
				 */
				T& value(){ return this->get(); }

				/**
				 * Get an immutable pointer to the contained value
				 * @return a realConst(T*) to the value
				 * @throws scl::exceptions::EmptyOptionalAccess if there's no value
				 */
				realConst(T*) ptr() const{ return &(this->get()); }

				/**
				 * Get a mutable pointer to the contained value
				 * @return a T* to the value
				 * @throws scl::exceptions::EmptyOptionalAccess if there's no value
				 */
				T* ptr(){ return &(this->get()); }

				/**
				 * Automatic bool conversion
				 * @return TRUE if there's a value, FALSE otherwise
				 */
				operator bool() const{ return this->hasValue(); }

				/**
				 * Access to the value
				 * @return a const& to the value stored
				 * @throws scl::exceptions::EmptyOptionalAccess if there's no value
				 */
				const T& operator*() const{ return this->get(); }

				/**
				 * Mutable access to the value
				 * @return a reference to the value stored
				 * @throws scl::exceptions::EmptyOptionalAccess if there's no value
				 */
				T& operator*(){ return this->get(); }

				/**
				 * Get an immutable pointer to the stored value
				 * @return a realConst T* to the value stored
				 * @throws scl::exceptions::EmptyOptionalAccess if there's no value
				 */
				realConst(T*) operator->() const{ return this->ptr(); }

				/**
				 * Get a mutable pointer to the stored value
				 * @return a T* to the value stored
				 * @throws scl::exceptions::EmptyOptionalAccess if there's no value
				 */
				T* operator->(){ return this->ptr(); }

				/**
				 * Retrieves the value if there's one or return the default value provided
				 * @param defaultValue being the value to return if there's no values
				 * @return a copy of the stored value
				 */
				T orElse(const T& defaultValue) const{
					try{
						return this->get();
					}catch(exceptions::EmptyOptionalAccess&){
						return defaultValue;
					}
				}

				/**
				 * Tries to retrieve the value, throws the given exception if there's none
				 * @tparam E being the exception type
				 * @param e being the exception to throw if there's no value
				 * @return a const& to the value
				 */
				template <class E>
				const T& orThrow(const E& e) const{
					try{
						return this->get();
					}catch(exceptions::EmptyOptionalAccess&){
						throw e;
					}
				}

				/**
				 * Maps this Optional<T> to an Optional<U> via the provided mapper function (T -> U)
				 * @tparam U being the value type for the mapped optional
				 * @tparam F being the type of the mapper function (auto deduction)
				 * @param mapper being the mapper function to use to map values
				 * @return an Optional<U>
				 */
				template <class U, class F>
				Optional<U> map(F mapper) const{
					try{
						const T& _ = this->get();
						return mapper(_);
					}catch(exceptions::EmptyOptionalAccess&){
						return none;
					}
				}

				/**
				 * Alias for Optional::map
				 */
				template <class U, class F>
				Optional<U> mapTo(F mapper) const{ return this->map<U>(mapper); }

				/**
				 * Filters the value accoding to the given predicate
				 * @tparam F being the type of predicate (auto deduction)
				 * @param predicate being the predicate used to determine whether or not it should keep the value
				 * @return a new optional that might not contain the original value
				 */
				template <class F>
				Optional<T> filter(F predicate) const{
					try {
						const T& _ = this->get();
						return predicate(_) ? Optional<T>{_} : Optional<T>{};
					}catch(exceptions::EmptyOptionalAccess&){
						return none;
					}
				}

			public:
				bool operator==(None) const{ return !this->hasValue(); }
				friend bool operator==(None, const Optional& o){ return o == none; }

				bool operator<(None) const{ return false; }
				friend bool operator<(None, const Optional&){ return true; }

				bool operator<=(None) const{ return (*this) == none; }
				friend bool operator<=(None, const Optional&){ return true; }

				bool operator>(None) const{ return true; }
				friend bool operator>(None, const Optional&){ return false; }

				bool operator>=(None) const{ return true; }
				friend bool operator>=(None, const Optional& o){ return o <= none; }

				bool operator!=(None) const{ return !((*this) == none); }
				friend bool operator!=(None, const Optional& o){ return o != none; }

#define SCL_TPL template <class U, class = META::enable_if_t<\
	!META::is_same<U, None>()\
	&& !META::is_same<U, Optional>()\
	&& !META::is_instance<scl::utils::Optional, U>()\
>>
				SCL_TPL
				bool operator==(const U& t) const{ return this->hasValue() && this->value() == t; }
				SCL_TPL
				bool operator!=(const U& t) const{ return !((*this) == t); }
				SCL_TPL
				bool operator<(const U& t) const{ return !this->hasValue() || this->value() < t; }
				SCL_TPL
				bool operator<=(const U& t) const{ return (*this) == t || (*this) < t; }
				SCL_TPL
				bool operator>(const U& t) const{ return !((*this) <= t); }
				SCL_TPL
				bool operator>=(const U& t) const{ return !((*this) < t); }

				SCL_TPL
				friend bool operator==(const U& t, const Optional& o){ return o == t; }
				SCL_TPL
				friend bool operator!=(const U& t, const Optional& o){ return o != t; }
				SCL_TPL
				friend bool operator<(const U& t, const Optional& o){ return o > t; }
				SCL_TPL
				friend bool operator<=(const U& t, const Optional& o){ return o >= t; }
				SCL_TPL
				friend bool operator>(const U& t, const Optional& o){ return o < t; }
				SCL_TPL
				friend bool operator>=(const U& t, const Optional& o){ return o <= t; }
#undef SCL_TPL

#define SCL_TPL template<class U/*, class = META::enable_if_t<!META::is_same<U,T>()>*/>
				SCL_TPL
				bool operator==(const Optional<U>& o) const{
					if(!this->hasValue())
						return !o.hasValue();

					return o.hasValue() && this->value() == o.value();
				}

				SCL_TPL
				bool operator!=(const Optional<U>& o) const{ return !((*this) == o); }

				SCL_TPL
				bool operator<(const Optional<U>& o) const{
					if(!this->hasValue())
						return o.hasValue();

					return o.hasValue() && this->value() < o.value();
				}

				SCL_TPL
				bool operator<=(const Optional<U>& o) const{ return (*this) == o || (*this) < o; }

				SCL_TPL
				bool operator>(const Optional<U>& o) const{ return o < (*this); }

				SCL_TPL
				bool operator>=(const Optional<U>& o) const{ return o <= (*this); }
#undef SCL_TPL
		};
	}
}