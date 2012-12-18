#ifndef _THREAD_COMM_QUEUE_H_
#define _THREAD_COMM_QUEUE_H_

#include <queue>
#include <pthread.h>

template <typename T>
class ThreadCommQueue
{
	public:
		ThreadCommQueue();
		~ThreadCommQueue();
		
		T* Pop();
		void Push(T* value);
		int Size();
		bool Alive();
		void Kill();
	private:
		std::queue<T*> mQueue;
		pthread_mutex_t	mLock;
		pthread_cond_t mCondition;
		bool mAlive;
};

template <typename T> ThreadCommQueue<T>::ThreadCommQueue()
{
	pthread_mutex_init( &mLock, NULL );
	pthread_cond_init( &mCondition, NULL );
	mAlive = true;
}

template <typename T> ThreadCommQueue<T>::~ThreadCommQueue()
{
	pthread_mutex_destroy( &mLock );
	pthread_cond_broadcast( &mCondition );
	pthread_cond_destroy( &mCondition );
}

template <typename T> T* ThreadCommQueue<T>::Pop()
{
	T* ret = NULL;

	pthread_mutex_lock( &mLock );
	if( !mAlive )
	{
		pthread_mutex_unlock( &mLock );
		return NULL;
	}
	
	if( mQueue.size() < 1 )
	{
		//Wait for data to be available from the queue
		pthread_cond_wait( &mCondition, &mLock );
	}
	
	//Check again in case the broadcast comes from Kill
	if( mQueue.size() > 0 )
	{
		//Get the data from the front of the queue
		ret = mQueue.front();
		mQueue.pop();
	}
	
	pthread_mutex_unlock( &mLock );
	
	return ret;
}

template <typename T> void ThreadCommQueue<T>::Push(T* value)
{
	pthread_mutex_lock( &mLock );
	mQueue.push(value);
	pthread_cond_signal( &mCondition );
	pthread_mutex_unlock( &mLock );
}

template <typename T> int ThreadCommQueue<T>::Size()
{
	int size;
	pthread_mutex_lock( &mLock );
	size = mQueue.size();
	pthread_mutex_unlock( &mLock );
	return size;
}

template <typename T> bool ThreadCommQueue<T>::Alive()
{
	bool is_alive;
	pthread_mutex_lock( &mLock );
	is_alive = mAlive;
	pthread_mutex_unlock( &mLock );
	return is_alive;
}

template <typename T> void ThreadCommQueue<T>::Kill()
{
	pthread_mutex_lock( &mLock );
	mAlive = false;
	pthread_cond_signal( &mCondition );
	pthread_mutex_unlock( &mLock );
}

#endif //_THREAD_COMM_QUEUE_H_

