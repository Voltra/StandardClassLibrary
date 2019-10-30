#pragma once
#include <mutex>
#include <queue>
#include <deque>
#include <condition_variable>
#include <chrono>
#include <tuple>
#include <scl/exceptions/NonNullViolation.h>

namespace scl{
	namespace async{
		template <class, class, template<class> class, class>
		class Channel;

		namespace details{
			template <class>
			class ChannelSender;

			template <class>
			class ChannelReceiver;

			/**
			 * Traits for the implementation of the transport of information in a scl::async::Channel
			 * @tparam Channel being the channel type
			 */
			template <class Channel>
			struct channel_transport_traits{
				/**
				 * @typedef channel_type
				 * The type of the channel
				 */
				using channel_type = Channel;

				/**
				 * @typdef sender_type
				 * The type of the sender
				 */
				using sender_type = typename channel_type::sender_type;

				/**
				 * @typedef receiver_type
				 * The type of the receiver
				 */
				using receiver_type = typename channel_type::receiver_type;

				/**
				 * @typedef type
				 * The type of transport payload
				 */
				using type = std::tuple<sender_type, receiver_type>;
//				using type = std::pair<sender_type, receiver_type>;

				/**
				 * Create a transport payload from a sender and a receiver
				 * @param sender being a sender for the channel
				 * @param receiver being  a receiver for the channel
				 * @return the corresponding transport payload
				 */
				static type factory(sender_type sender, receiver_type receiver){
					return type{std::move(sender), std::move(receiver)};
//					return std::make_pair(std::move(sender), std::move(receiver));
				}
			};

			/**
			 * Traits for actors that interact with the channel
			 * @tparam ChannelActor being the type of the actor
			 */
			template <class ChannelActor>
			struct channel_actor_traits{
				/**
				 * @typedef actor_type
				 * The type of the actor
				 */
				using actor_type = ChannelActor;

				/**
				 * @typedef channel_type
				 * The type of the channel
				 */
				using channel_type = typename actor_type::channel_type;

				/**
				 * @typedef lock_type
				 * The type of lock used by the channel
				 */
				using lock_type = typename channel_type::lock_type;

				/**
				 * @typedef guard_type
				 * @tparam L being the type of lock
				 * The type of guard used to protect the lock
				 */
				template <class L>
				using guard_type = typename channel_type::template guard_type<L>;

				/**
				 * @typedef value_type
				 * The type of values transmitted through channels
				 */
				using value_type = typename channel_type::value_type;

				/**
				 * @typedef queue_type
				 * The type of the queue used in the channel
				 */
				using queue_type = typename channel_type::queue_type;

				/**
				 * A (safe) pointer to the actor
				 */
				actor_type* actor;

				/**
				 * The handle to the channel's lock
				 */
				std::unique_lock<lock_type> lock_;

				/**
				 * Construct a trait from a pointer to its actor
				 * @param act being a (safe) pointer to the actor
				 * @throws scl::exceptions::NonNullViolation
				 */
				explicit channel_actor_traits(actor_type* act) : actor{act} {
					if(!act)
						throw scl::exceptions::NonNullViolation{};

					this->lock_ = {this->actor->channel->lock, std::defer_lock};
				}

				/**
				 * Lock the channel
				 */
				void lock(){
					this->lock_.lock();
				}

				/**
				 * Unlock the channel
				 */
				void unlock(){
					this->lock_.unlock();
				}

				/**
				 * Notify the channel
				 */
				void notify(){
					this->actor->channel->condition.notify_one();
				}

				/**
				 * Wait for the channel
				 *
				 * @warning locks, will need to unlock afterward
				 */
				void wait(){
					this->lock();
					this->actor->channel->condition.wait(this->lock_);
				}

				/**
				 * Wait for the channel until a condition is met
				 * @tparam Pred being the type of the predicate
				 * @param pred being the predicate to match
				 *
				 * @warning locks, will need to unlock afterward
				 */
				template <class Pred>
				void waitUntil(Pred&& pred){
					this->lock();
					this->actor->channel->condition.wait(this->lock_, pred);
				}

				/**
				 * Wait for a specific duration for the channel
				 * @tparam Rep being the time representation type
				 * @tparam Period being the time period type
				 * @param time being the time to wait
				 * @return TIMEOUT if it waited for time, NO_TIMEOUT otherwise
				 *
				 * @warning locks, will need to unlock afterward
				 */
				template <class Rep, class Period>
				std::cv_status waitFor(const std::chrono::duration<Rep, Period>& time){
					this->lock();
					return this->actor->channel->condition.wait_for(this->lock_, time);
				}

				/**
				 * Wait for a given period of time until the predicate is satisfied
				 * @tparam Rep being the time representation type
				 * @tparam Period being the time period type
				 * @param time being the time to wait
				 * @param pred being the predicate to satisfy
				 * @return TRUE if the predicate was satisified, FALSE otherwise
				 *
				 * @warning locks, will need to unlock afterward
				 */
				template <class Rep, class Period, class Pred>
				bool waitFor(const std::chrono::duration<Rep, Period>& time, Pred&& pred){
					this->lock();
					return this->actor->channel->condition.wait_for(this->lock_, time, pred);
				}
			};
		}

		/**
		 * A channel to use for asynchronous communication of data in a queued manner
		 * @tparam T being the type of values
		 * @tparam Lock being the type of lock to use
		 * @tparam Guard being the type of guard to use
		 * @tparam Container being the container to use for the queue
		 */
		template <class T, class Lock = std::mutex, template<class> class Guard = std::lock_guard, class Container = std::deque<T>>
		class Channel{
			public:
				/**
				 * @typdef lock_type
				 * The type of locks used in the channel
				 */
				using lock_type = Lock;

				/**
				 * @typedef guard_type
				 * @tparam L being the type of lock
				 * The type of lock guard used in the channel
				 */
				template <class L>
				using guard_type = Guard<L>;

				/**
				 * @typdef queue_type
				 * The type of the channel's queue
				 */
				using queue_type = std::queue<T, Container>;

				/**
				 * @typedef value_type
				 * The type of values exposed through the channel
				 */
				using value_type = typename queue_type::value_type;

				/**
				 * @typedef size_type
				 * The type used to quantize the size of the queue
				 */
				using size_type = typename queue_type::size_type;

				/**
				 * @typedef sender_type
				 * The type of senders for the channel
				 */
				using sender_type = details::ChannelSender<Channel>;

				/**
				 * @typedef receiver_type
				 * The type of receivers for the channel
				 */
				using receiver_type = details::ChannelReceiver<Channel>;

				/**
				 * @typedef transport_traits
				 * The type of the transport traits for the channel
				 */
				using transport_traits = details::channel_transport_traits<Channel>;

				/**
				 * @typedef transport_type
				 * The type of the transport payload
				 */
				using transport_type = typename transport_traits::type;

				friend sender_type;
				friend receiver_type;
				friend details::channel_actor_traits<sender_type>;
				friend details::channel_actor_traits<receiver_type>;

				template <size_t I, class U, class L, template<class> class G, class C>
				friend auto std::get(Channel<U, L, G, C>& chan) -> typename std::tuple_element<I, typename Channel<U, L, G, C>::transport_type>::type;

			protected:
				/**
				 * The lock being used for the channel's queue
				 */
				mutable lock_type lock;

				/**
				 * The condition used to wait for the channel's queue
				 */
				mutable std::condition_variable_any condition;

				/**
				 * The underlying queue used to store data
				 */
				queue_type queue;

				/**
				 * Implementation details to get a receiver and sender for this channel
				 * @return the transport payload
				 */
				transport_type interface(){
					//TODO: Wonder if it would be better to only have one of each
					return transport_traits::factory(this->sender(), this->receiver());
				}

			public:
				/**
				 * Construct a channel
				 */
				Channel() : lock{}, condition{}, queue{} {
				}

				Channel(const Channel&) = delete;
				Channel(Channel&&) = delete;
				Channel& operator=(const Channel&) = delete;
				Channel& operator=(Channel&&) = delete;

				/**
				 * Retrieve the size of the queue
				 * @return the size of the queue
				 */
				size_type size() const{
					guard_type<lock_type> _{lock};
					return this->queue.size();
				}

				/**
				 * Alias for Channel::size
				 */
				size_type length() const{
					return this->size();
				}

				/**
				 * Determine whether or not the channel is currently empty
				 * @return TRUE if it is, FALSE otherwise
				 */
				bool isEmpty() const{
					return this->size() <= 0;
				}

				/**
				 * Get a receiver for this channel
				 * @return the receiver
				 */
				receiver_type receiver(){
					return receiver_type{this};
				}

				/**
				 * Get a sender for this channel
				 * @return the sender
				 */
				sender_type sender(){
					return sender_type{this};
				}
		};

		namespace details{
			/**
			 * Class used to send data to a Channel
			 * @tparam Chan being the channel type
			 */
			template <class Chan>
			class ChannelSender{
				public:
					/**
					 * @typedef channel_type
					 * The type of the channel
					 */
					using channel_type = Chan;

					/**
					 * @typedef sender_traits
					 * The traits of the sender
					 */
					using sender_traits = channel_actor_traits<ChannelSender>;

					/**
					 * @typedef lock_type
					 * The type of lock used in the sender
					 */
					using lock_type = typename sender_traits::lock_type;

					/**
					 * @typedef guard_type
					 * The type of guards used in the sender
					 */
					template <class L>
					using guard_type = typename sender_traits::template guard_type<L>;

					/**
					 * @typedef queue_type
					 * The type of queue used in the channel
					 */
					using queue_type = typename sender_traits::queue_type;

					/**
					 * @typedef value_type
					 * The type of values used in the channel
					 */
					using value_type = typename sender_traits::value_type;

					friend sender_traits;

				protected:
					/**
					 * A (safe) pointer to the channel
					 */
					Chan* channel;

					/**
					 * The traits for the sender
					 */
					sender_traits traits;

				public:
					/**
					 * Construct a sender from a (safe) pointer to its channel
					 */
					explicit ChannelSender(Chan* chan) : channel{chan}, traits{this} {
						if(!chan)
							throw scl::exceptions::NonNullViolation{};
					}

				protected:
					/**
					 * Implementation details to push data onto the channel
					 * @tparam Args being the type of the arguments for the constructor
					 * @param args being the arguments for the constructor
					 * @return a reference to this ChannelSender
					 */
					template <class... Args>
					ChannelSender& doPush(Args&&... args){
						guard_type<sender_traits> _{this->traits};
						this->channel->queue.emplace(std::forward<Args>(args)...);
						this->traits.notify();
						return *this;
					}

				public:
					/**
					 * Push by copy
					 * @param value being the value to copy
					 * @return a reference to this ChannelSender
					 */
					ChannelSender& push(const value_type& value){
						return this->doPush(value);
					}

					/**
					 * Push by move
					 * @param value being the value to move
					 * @return a reference to this ChannelSender
					 */
					ChannelSender& push(value_type&& value){
						return this->doPush(std::move(value));
					}

					/**
					 * Construct and push in-place the value
					 * @tparam Args being the types of arguments for the constructor
					 * @param args being the arguments for the constructor
					 * @return a reference to this ChannelSender
					 */
					template <class... Args>
					ChannelSender& pushEmplace(Args&&... args){
						return this->doPush(std::forward<Args&&>(args)...);
					}

					/**
					 * Alias for ChannelSender::push
					 */
					ChannelSender& queue(const value_type& value){ return this->push(value); }
					ChannelSender& queue(value_type&& value){ return this->push(std::move(value)); }
					ChannelSender& enqueue(const value_type& value){ return this->push(value); }
					ChannelSender& enqueue(value_type&& value){ return this->push(std::move(value)); }
					ChannelSender& send(const value_type& value){ return this->push(value); }
					ChannelSender& send(value_type&& value){ return this->push(std::move(value)); }

					ChannelSender& operator<<(const value_type& value){ return this->push(value); }
					ChannelSender& operator<<(value_type&& value){ return this->push(value); }
			};

			template <class Chan>
			class ChannelReceiver{
				public:
					using channel_type = Chan;
					using receiver_traits = channel_actor_traits<ChannelReceiver>;
					using lock_type = typename receiver_traits::lock_type;

					template <class L>
					using guard_type = typename receiver_traits::template guard_type<L>;
					using queue_type = typename receiver_traits::queue_type;
					using value_type = typename receiver_traits::value_type;
					using optional_type = scl::utils::Optional<value_type>;

					friend receiver_traits;

				protected:
					Chan* channel;
					receiver_traits traits;

				public:
					explicit ChannelReceiver(Chan* chan) : channel{chan}, traits{this} {
						if(!chan)
							throw scl::exceptions::NonNullViolation{};
					}

				public:
					value_type pop(){
						this->traits.waitUntil([&]{
							return !this->channel->queue.empty();
						});

						auto value = this->channel->queue.front();
						this->channel->queue.pop();
						this->traits.unlock(); //TODO: Check if necessary
						return value;
					}

					template <class Rep, class Period>
					optional_type tryPop(const std::chrono::duration<Rep, Period>& duration){
						bool isEmpty = this->traits.waitFor(duration, [&]{
							return !this->channel->queue.empty();
						});

						if(isEmpty)
							return optional_type{};

						auto value = this->channel->queue.front();
						this->channel->queue.pop();
						this->traits.unlock();
						return optional_type{std::move(value)};
					}

					/**
					 * Alias for ChannelReceiver::receive
					 */
					value_type dequeue(){ return this->pop(); }
					value_type receive(){ return this->pop(); }

					/**
					 * Alias for ChannelReceiver::tryPop
					 */
					template <class Rep, class Period>
					optional_type tryDeque(const std::chrono::duration<Rep, Period>& duration){
						return this->tryPop(duration);
					}
					template <class Rep, class Period>
					optional_type tryReceive(const std::chrono::duration<Rep, Period>& duration){
						return this->tryPop(duration);
					}

					ChannelReceiver& operator>>(value_type& value){
						value = this->receive();
						return *this;
					}
			};
		}
	}
}

namespace std{
	template <size_t I, class T, class Lock, template<class> class Guard, class Container>
	auto get(scl::async::Channel<T, Lock, Guard, Container>& channel) -> typename std::tuple_element<I, typename scl::async::Channel<T, Lock, Guard, Container>::transport_type>::type {
		return std::get<I>(channel.interface());
	}

	template <class T, class Lock, template<class> class Guard, class Container>
	struct tuple_size<scl::async::Channel<T, Lock, Guard, Container>>
			: std::tuple_size<typename scl::async::Channel<T, Lock, Guard, Container>::transport_type>{
	};
}