#pragma once

#include <utility>
#include <scl/macros.h>
#include <scl/exceptions/InvalidEitherAccess.h>
#include <scl/tools/meta/enable_if.h>
#include <scl/tools/meta/type_check.h>
#include <scl/tools/meta/void_t.h>
#include <scl/tools/meta/is_same.h>
#include <scl/utils/Optional.h>

namespace scl{
	namespace utils{
		/**
		 * Alternative type that stores one or the other
		 * @tparam Lhs being the left value type
		 * @tparam Rhs being the right value type
		 * @warning Either defines move and/or copy semantics only if both
		 * the underlying types do so
		 */
		template <class Lhs, class Rhs>
		class Either{
			public:
				using left_type = Lhs;
				using right_type = Rhs;

			protected:
				static constexpr bool is_copyable(){
					return META::is_copyable<Lhs>()
					&& META::is_copyable<Rhs>();
				}

				static constexpr bool is_movable(){
					return META::is_movable<Lhs>()
					&& META::is_movable<Rhs>();
				}

				struct payload_t final{
					Optional<Lhs> left = {};
					Optional<Rhs> right = {};

					payload_t() = default;
					~payload_t() = default;

					template <class = META::enable_if_t<is_movable()>>
					payload_t(payload_t&& p) : left{std::move(p.left)}, right{std::move(p.right)} {
					}

					template <class = META::enable_if_t<is_copyable()>>
					payload_t(const payload_t& p) : left{p.left}, right{p.right} {
					}
				};

				/**
				 * @struct lhs_tag
				 * Tag used to construct the LHS member (in case both are convertible types)
				 */
				struct lhs_tag final{};

				/**
				 * @struct rhs_tag
				 * Tag used to construct the RHS member (in case both are convertible types)
				 */
				struct rhs_tag final{};

				/**
				 * @var lhs
				 * Determines whether or ot the LHS member is the current alternative
				 */
				bool lhs;

				/**
				 * @var payload
				 * Stores the actual alternative payload
				 */
				payload_t payload = {};


			protected:
				/**
				 * Construct the LHS alternative (rvalue)
				 * @param lhs being the LHS value to construct from
				 */
				 template <class = META::enable_if_t<is_movable()>>
				Either(lhs_tag, Lhs&& lhs) : lhs{true} {
					this->payload.left = std::move(lhs);
				}

				/**
				 * Construct the LHS alternative (lvalue)
				 * @param lhs being the LHS value to construct from
				 */
				 template <class = META::void_t<META::enable_if_t<
					 META::is_copyable<Lhs>()
				 >>>
				Either(lhs_tag, const Lhs& lhs) : lhs{true} {
					this->payload.left = lhs;
				}

				/**
				 * Construct the RHS alternative (rvalue)
				 * @param rhs being the RHS value to construct from
				 */
				template <class = META::void_t<META::enable_if_t<
					META::is_movable<Rhs>()
				>>>
				Either(rhs_tag, Rhs&& rhs) : lhs{false} {
					this->payload.right = std::move(rhs);
				}

				/**
				 * Construct the RHS alternative (lvalue)
				 * @param rhs being the RHS value to construct from
				 */
				template <class = META::void_t<META::enable_if_t<
					META::is_copyable<Rhs>()
				>>>
				Either(rhs_tag, const Rhs& rhs) : lhs{false} {
					this->payload.right = rhs;
				}

				/**
				 * Handy visitor that does nothing on the LHS
				 * @param lhs being the LHS value of the alternative
				 */
				static void leftVoidVisitor(const Lhs& lhs){}

				/**
				 * Handy visitor that does nothing on the RHS
				 * @param rhs being the RHS value of the alternative
				 */
				static void rightVoidVisitor(const Rhs& rhs){}

			public:
				Either() = delete;

				template <class = META::enable_if_t<is_copyable()>>
				Either(const Either& other) : payload{}, lhs{other.lhs} {
					if(lhs)
						this->payload.left = other.getLeft();
					else
						this->payload.right = other.getRight();
				}

				template <class = META::enable_if_t<is_copyable()>>
				Either& operator=(const Either& other){
					this->lhs = other.lhs;
					if(lhs)
						this->payload.left = other.getLeft();
					else
						this->payload.right = other.getRight();

					return *this;
				}

				template <class = META::enable_if_t<is_movable()>>
				Either(Either&& other) noexcept :  payload{}, lhs{other.lhs} {
					if(lhs)
						this->payload.left = std::move(other.payload.left);
					else
						this->payload.right = std::move(other.payload.right);
				}

				template <class = META::enable_if_t<is_movable()>>
				Either& operator=(Either&& other) noexcept{
					this->lhs = other.lhs;
					if(lhs)
						this->payload.left = std::move(other.payload.left);
					else
						this->payload.right = std::move(other.payload.right);

					return *this;
				};

				/**
				 * Construct the LHS
				 * @param lhs being the value to construct from
				 * @return a Either<Lhs, Rhs> where the Lhs is the active alternative
				 */
				 template <class L>
				static Either left(L&& lhs){
					return Either{lhs_tag{}, std::forward<L>(lhs)};
				}

				/**
				 * Alias for Either::left
				 */
				 template <class L>
				static Either Left(L&& lhs){
					return left(std::forward<L>(lhs));
				}

				/**
				 * Construct the RHS
				 * @param rhs being the value to construct from
				 * @return a Either<Lhs, Rhs> where the Rhs is the active alternative
				 */
				 template <class R>
				static Either right(R&& rhs){
					return Either{rhs_tag{}, std::forward<R>(rhs)};
				}

				/**
				 * Alias for Either::right
				 */
				 template <class R>
				static Either Right(R&& rhs){
					return right(std::forward<R>(rhs));
				}

				/**
				 * Determines whether or not LHS is the active alternative
				 * @return TRUE if it is, FALSE otherwise
				 */
				bool hasLeft() const{ return this->lhs; }

				/**
				 * Determines whether or not RHS is the active alternative
				 * @return TRUE if it is, FALSE otherwise
				 */
				bool hasRight() const{ return !this->hasLeft(); }

				/**
				 * Attempts to get the LHS value
				 * @return a const& to the LHS value
				 * @throws exceptions::InvalidEitherAccess if LHS is not the active alternative
				 */
				const Lhs& getLeft() const{
					if(!this->hasLeft())
						throw exceptions::InvalidEitherAccess::leftWhenRight();

					return this->payload.left.get();
				}

				/**
				 * Attempts to get the RHS value
				 * @return a const& to the RHS value
				 * @throws exceptions::InvalidEitherAccess if RHS is not the active alternative
				 */
				const Rhs& getRight() const{
					if(!this->hasRight())
						throw exceptions::InvalidEitherAccess::rightWhenLeft();

					return this->payload.right.get();
				}

				/**
				 * Visit this Either
				 * @tparam LeftVisitor being the type of the visitor for the LHS
				 * @tparam RightVisitor being the type of the visitor for the RHS
				 * @param visitLeft being the visitor for the LHS
				 * @param visitRight being the visitor for the RHS
				 * @return a reference to this Either
				 */
				template <class LeftVisitor, class RightVisitor>
				const Either& visit(LeftVisitor visitLeft, RightVisitor visitRight) const{
					if(this->hasLeft()) {
						const Lhs& left = this->getLeft();
						visitLeft(left);
					}else {
						const Rhs& right = this->getRight();
						visitRight(right);
					}

					return *this;
				}

				/**
				 * Execute a callback if LHS is the active alternative
				 * @tparam LeftVisitor being the type of the callback
				 * @param visitLeft being the callback
				 * @return a reference to this Either
				 */
				template <class LeftVisitor>
				const Either& doIfLeft(LeftVisitor visitLeft) const{
					return this->visit(visitLeft, Either::rightVoidVisitor);
				}

				/**
				 * Execute a callback if RHS is the active alternative
				 * @tparam RightVisitor being the type of the callback
				 * @param visitRight being the callback
				 * @return a reference to this Either
				 */
				template <class RightVisitor>
				const Either& doIfRight(RightVisitor visitRight) const{
					return this->visit(Either::leftVoidVisitor, visitRight);
				}

				/**
				 * Maps the LHS if there's any
				 * @tparam NewLhs being the new LHS value type
				 * @tparam Mapper being Lhs -> NewLhs
				 * @param mapper being the mapper function
				 * @return a correctly mapper Either<NewLhs, Rhs>
				 */
				template <class NewLhs, class Mapper>
				Either<NewLhs, Rhs> mapLeftTo(Mapper mapper) const{
					if(this->hasLeft()){
						const Lhs& left = this->getLeft();
						return Either<NewLhs, Rhs>::/*template*/ Left/*<NewLhs>*/(mapper(left));
					}

					return Either<NewLhs, Rhs>::Right(this->getRight());
				}

				/**
				 * Maps the RHS if there's any
				 * @tparam NewRhs being the new RHS value type
				 * @tparam Mapper being Rhs -> NewRhs
				 * @param mapper being the mapper function
				 * @return a correctly mapper Either<Lhs, NewRhs>
				 */
				template <class NewRhs, class Mapper>
				Either<Lhs, NewRhs> mapRightTo(Mapper mapper) const{
					if(this->hasRight()){
						const Rhs& right = this->getRight();
						return Either<Lhs, NewRhs>::/*template*/ Right/*<NewRhs>*/(mapper(right));
					}

					return Either<Lhs, NewRhs>::Left(this->getLeft());
				}

				/**
				 * Maps both alternatives
				 * @tparam NewLhs being the new LHS value type
				 * @tparam NewRhs being the new RHS value type
				 * @tparam MapperLeft being Lhs -> NewLhs
				 * @tparam MapperRight being Rhs -> NewRhs
				 * @param mapLeft being the LHS mapper function
				 * @param mapRight being the RHS mapper function
				 * @return the mapped Either<NewLhs, NewRhs>
				 */
				template <class NewLhs, class NewRhs, class MapperLeft, class MapperRight>
				Either<NewLhs, NewRhs> mapTo(MapperLeft mapLeft, MapperRight mapRight) const{
					if(this->hasLeft()){
						const Lhs& left = this->getLeft();
						return Either<NewLhs, NewRhs>::/*template*/ Left/*<NewLhs>*/(mapLeft(left));
					}else{
						//this->hasRight()
						const Rhs& right = this->getRight();
						return Either<NewLhs, NewRhs>::/*template*/ Right/*<NewRhs>*/(mapRight(right));
					}
				}

				/**
				 * Tries to get the LHS value and fallback to default if not
				 * @param defaultValue being the default value to use if LHS is not the current alternative
				 * @return Lhs
				 */
				Lhs leftOr(Lhs&& defaultValue) const{
					try{
						return this->getLeft();
					}catch(exceptions::InvalidEitherAccess&){
						return std::forward<Lhs>(defaultValue);
					}
				}

				/**
				 * Tries to get the RHS value and fallback to default if not
				 * @param defaultValue being the default value to use if LHS is not the current alternative
				 * @return Rhs
				 */
				Rhs rightOr(Rhs&& defaultValue) const{
					try{
						return this->getRight();
					}catch(exceptions::InvalidEitherAccess&){
						return std::forward<Rhs>(defaultValue);
					}
				}
		};
	}
}