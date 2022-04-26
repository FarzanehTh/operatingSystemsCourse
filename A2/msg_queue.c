/**
 * CSC369 Assignment 2 - Message queue implementation.
 *
 * You may not use the pthread library directly. Instead you must use the
 * functions and types available in sync.h.
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "errors.h"
#include "list.h"
#include "msg_queue.h"
#include "ring_buffer.h"

// Message queue implementation backend
typedef struct mq_backend
{
	// Ring buffer for storing the messages
	ring_buffer buffer;

	// Reference count
	size_t refs;

	// Number of handles open for reads
	size_t readers;
	// Number of handles open for writes
	size_t writers;

	// Set to true when all the reader handles have been closed. Starts false
	// when they haven't been opened yet.
	bool no_readers;
	// Set to true when all the writer handles have been closed. Starts false
	// when they haven't been opened yet.
	bool no_writers;

	//TODO: add necessary synchronization primitives, as well as data structures
	//      needed to implement the msg_queue_poll() functionality
	mutex_t buffer_lock;
	cond_t empty;
	cond_t full;

	// Linked list
	struct list_head waiting_queue;

} mq_backend;

struct node
{
	/* data */
	struct msg_queue_pollfd *fd;
	struct list_entry entry;
	cond_t *cv;
	mutex_t *lock;
	int *signalled;
};

static int mq_init(mq_backend *mq, size_t capacity)
{
	if (ring_buffer_init(&mq->buffer, capacity) < 0)
	{
		return -1;
	}

    /* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
    return 0;
}

static void mq_destroy(mq_backend *mq)
{
	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */

	// cleanup remaining fields (synchronization primitives, etc.)
	cond_destroy(&mq->empty);
	cond_destroy(&mq->full);
	mutex_destroy(&mq->buffer_lock);
	list_destroy(&mq->waiting_queue);
}

// Message queue handle is a combination of the pointer to the queue backend and
// the handle flags. The pointer is always aligned on 8 bytes - its 3 least
// significant bits are always 0. This allows us to store the flags within the
// same word-sized value as the pointer by ORing the pointer with the flag bits.

/**
 * Signals all waiting threads in mq. Also updates revents with flags.
 * @param mq: The message queue implementation backend
 * @param flags: The flags for the specific operation that is going on, for example, MQPOLL_WRITABLE
 */
void signal_waiting_threads(mq_backend *mq, int flag, int flag_)
{
	/* signal all waiting polling threads*/
	struct list_head *waiting_queue_head = &(mq->waiting_queue);
	struct list_entry *pos;
    list_for_each(pos, waiting_queue_head) { 
        /* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */ 
    }
    cond_signal((node->cv));
    mutex_unlock((node->lock));
	}
	/* end signal all waiting polling threads */
}

// Get queue backend pointer from the queue handle
static mq_backend *get_backend(msg_queue_t queue)
{
	mq_backend *mq = (mq_backend *)(queue & ~ALL_FLAGS);
	assert(mq);
	return mq;
}

// Get handle flags from the queue handle
static int get_flags(msg_queue_t queue)
{
	return (int)(queue & ALL_FLAGS);
}

// Create a queue handle for given backend pointer and handle flags
static msg_queue_t make_handle(mq_backend *mq, int flags)
{
	assert(((uintptr_t)mq & ALL_FLAGS) == 0);
	assert((flags & ~ALL_FLAGS) == 0);
	return (uintptr_t)mq | flags;
}

static msg_queue_t mq_open(mq_backend *mq, int flags)
{
	++mq->refs;

	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */

	return make_handle(mq, flags);
}

// Returns true if this was the last handle
static bool mq_close(mq_backend *mq, int flags)
{
	assert(mq->refs != 0);
	assert(mq->refs >= mq->readers);
	assert(mq->refs >= mq->writers);

	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
	return false;
}

msg_queue_t msg_queue_create(size_t capacity, int flags)
{
	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
	return mq_open(mq, flags);
}

msg_queue_t msg_queue_open(msg_queue_t queue, int flags)
{
	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */

	return new_handle;
}

int msg_queue_close(msg_queue_t *queue)
{
	if (!queue || !*queue)
	{
		errno = EBADF;
		report_error("msg_queue_close\n");
		return -1;
	}

	mq_backend *mq = get_backend(*queue);

	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
	if ((mq->no_readers))
	{
	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
	}
	else if (mq->no_writers)
	{
		/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
		signal_waiting_threads(mq, MQPOLL_READABLE, MQPOLL_NOWRITERS);
	}

	mutex_unlock(&(mq->buffer_lock));

	*queue = MSG_QUEUE_NULL;
	return 0;
}

ssize_t msg_queue_read(msg_queue_t queue, void *buffer, size_t length)
{

	mq_backend *mq = get_backend(queue);

	// everything is good for reading
	// read length size from queue atomically
	mutex_lock(&(mq->buffer_lock));

	while (ring_buffer_used(&(mq->buffer)) <= 0)
	{
		/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
	}

	/* start reading one msg*/

	// check if there does not exist the size length in the queue
	size_t msg_size;
	if (!ring_buffer_peek(&mq->buffer, (void *)&msg_size, sizeof(size_t)))
	{
		errno = EMSGSIZE;
		cond_signal(&(mq->empty));
		mutex_unlock(&(mq->buffer_lock));
		return -1;
	}

	// check next mgs in the queue and check if the buffer can hold it
	// EMSGSIZE if size of mgs > length of buffer
	if (msg_size > length)
	{
		/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
	}

	// actually read size length from the queue
	ring_buffer_read(&(mq->buffer), &msg_size, sizeof(size_t));

	// actually read the msg
	ring_buffer_read(&(mq->buffer), buffer, msg_size);

    /* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
	/* end read */
	return (ssize_t)(msg_size);
}

int msg_queue_write(msg_queue_t queue, const void *buffer, size_t length)
{

	mq_backend *mq = get_backend(queue);

	// Check for errors with the data to be written
	if (length == 0)
	{
		// Zero length error
		errno = EINVAL;
		return -1;
	}
	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */

	mutex_lock(&(mq->buffer_lock));

	if (length + sizeof(size_t) > mq->buffer.size)
	{
		// Length too large for buffer
		cond_signal(&(mq->full));
		mutex_unlock(&(mq->buffer_lock));
		errno = EMSGSIZE;
		return -1;
	}

	// while (ring_buffer_free(&mq->buffer) >= mq->buffer.size)
	// Because our condition variable is when it is empty, we are only going to be writing when the buffer
	// is empty, so the condition in the while loop is when the free amount in the buffer is >= the size of the buffer
	while (ring_buffer_free(&mq->buffer) < length + sizeof(size_t))
	{
		/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
	}

	bool success = false;

	// Write the size to the buffer
	success = ring_buffer_write(&(mq->buffer), (const void *)&length, sizeof(size_t));

	// Check if there was an error
	if (!success)
	{
		errno = EMSGSIZE;
		cond_signal(&(mq->full));
		mutex_unlock(&(mq->buffer_lock));
		return -1;
	}

	// Write the data to the buffer
	success = ring_buffer_write(&(mq->buffer), buffer, length);

	if (!success)
	{
		errno = EMSGSIZE;
		cond_signal(&(mq->full));
		mutex_unlock(&(mq->buffer_lock));
		return -1;
	}

	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
	// Signal and release the lock as nothing went wrong
	mutex_unlock(&(mq->buffer_lock));
	return 0;
}


bool set_revents(int *revents, int events, mq_backend *mq)
{
	mutex_lock(&(mq->buffer_lock));
	bool ready_for_reading = ring_buffer_used(&(mq->buffer)) > 0;
	bool ready_for_writing = ring_buffer_free(&mq->buffer) > 0;

	bool ready = false;
	// check if reading event is requested
	if ((MQPOLL_READABLE & events) && ready_for_reading)
	{
		ready = true;
		// set its revents
		*revents = *revents | MQPOLL_READABLE;
	}
	// check if writing event is requested
	if ((MQPOLL_WRITABLE & events) && ready_for_writing)
	{
		ready = true;
		// set its revents
		*revents = *revents | MQPOLL_WRITABLE;
	}

	// check if no readers has occured
	if (mq->no_writers)
	{
		ready = true;
		// set its revents
		*revents = *revents | MQPOLL_NOWRITERS;
	}

	// check if no writers has occured
	if (mq->no_readers)
	{
		ready = true;
		// set its revents
		*revents = *revents | MQPOLL_NOREADERS;
	}
	mutex_unlock(&(mq->buffer_lock));
	return ready;
}


int check_waiting_nodes(msg_queue_pollfd *fds, size_t nfds, struct list_entry **entries)
{
	int sum = 0;
	int t = 0;
	for (unsigned int i = 0; i < nfds; i++)
	{
		if (fds[i].queue != MSG_QUEUE_NULL)
		{
			/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */

			// delete this node
			list_del(&(Q->waiting_queue), entries[t]);
			free(node);
			t++;
			mutex_unlock(&(Q->buffer_lock));
		}
	}
	return sum;
}


/**
 * Checks to see if any revents have already occured in this queue
 * @param fd the file descriptor for this queue
 * @param queue the queue
 * @return Returns 1 if revents occured, 0 otherwise
 */
int check_if_signalled(msg_queue_pollfd *fd, msg_queue_t *queue)
{
	int events = fd->events;
	int *revents = &(fd->revents);
	mq_backend *mq = get_backend(*queue);
	if (set_revents(revents, events, mq))
	{
		return 1;
	}
	return 0;
}


/** msg_queue_poll() is called by every reader/writer thread that wants to poll. 
 * fds is an array of msg_queue_pollfd structs, one for each queue, and nfds is the number of our queues. **/
int msg_queue_poll(msg_queue_pollfd *fds, size_t nfds)
{
	int sum = 0;

	if (nfds == 0)
	{
		// No events are subscribed to
		errno = EINVAL;
		return -1;
	}
	size_t num_null = 0;

	/* STEP 1: see if any events for requested queues have already occurred and set their revents */
	for (unsigned int i = 0; i < nfds; i++)
	{
		msg_queue_t queue = fds[i].queue;
		if (queue != MSG_QUEUE_NULL)
		{
			int events = fds[i].events;
			// Check for invalid events field
			if (events == 0)
			{
				errno = EINVAL;
				report_error("poll: invalid events field = %d\n", events);
				return -1;
			}
			else if (!((events & MQPOLL_NOWRITERS) || (events & MQPOLL_WRITABLE) || (events & MQPOLL_NOREADERS) || (events & MQPOLL_READABLE)))
			{
				errno = EINVAL;
				report_error("poll: invalid events field = %d\n", events);
				return -1;
			}

			// Check for impossible event fields
			if ((events & MQPOLL_READABLE) & (events & MQPOLL_NOREADERS))
			{
				// Asking for readable when there are no readers
				errno = EINVAL;
				report_error("poll: impossible events field = %d\n", events);
				return -1;
			}
			else if ((events & MQPOLL_WRITABLE) & (events & MQPOLL_NOWRITERS))
			{
				errno = EINVAL;
				report_error("poll: impossible events field = %d\n", events);
				return -1;
			}

			// events were ok, now see if any of them is ready in this queue
			sum += check_if_signalled(&(fds[i]), &(fds[i].queue));
		}
		else
		{
			// We need to return an error if all of these are null
			int *revents = &(fds[i].revents);
			int *events = &(fds[i].events);
			*revents = 0;
			*events = 0;
			num_null++;
		}
	}

	// If all the queues were null so we have to return an error
	if (num_null == nfds)
	{
		errno = EINVAL;
		report_error("poll: All the queues (num = %d) were null\n", nfds);
		return -1;
	}

	// return if any events already occurred
	if (sum) return sum;

	/* STEP 2: None of the requested queues were ready, so create a waiting node in each queue's LL. i.e: subscribe to that queue.*/

	// So create one signle CV for this polling thread(i.e reader/writer thread that has called this poll function) to save in its nodes
	cond_t polling_thread_cv;
	cond_init(&polling_thread_cv);

	mutex_t lock;
	mutex_init(&lock);

	int signalled = 0;

	struct list_entry *entries[nfds - num_null];
	int t = 0;
	for (unsigned int i = 0; i < nfds; i++)
	{
		msg_queue_pollfd *fd = &(fds[i]);
		msg_queue_t queue = fds[i].queue;
		// create a node for this polling thread
		if (queue != MSG_QUEUE_NULL)
		{
			mq_backend *mq = get_backend(queue);
			mutex_lock(&(mq->buffer_lock));
			struct node *LL_node = malloc(sizeof(struct node));
			if (LL_node == NULL)
			{
				sum = check_waiting_nodes(fds, i , entries);
				if (sum) return sum;
				mutex_unlock(&(mq->buffer_lock));
				report_error("poll: Malloc failure\n");
				return -1;
			}
			LL_node->fd = fd;
			LL_node->signalled = &signalled;
			LL_node->cv = &polling_thread_cv;
			LL_node->lock = &lock;
			entries[t] = &(LL_node->entry);
			t++;

			// add this node to waiting queue for this queue atomically
			list_add_tail(&(mq->waiting_queue), &(LL_node->entry));
			mutex_unlock(&(mq->buffer_lock));
		}
	}

	/* STEP 3: It is possible that an event has occured during the preceeding loop, as a result, we need to search for it
	otherwise, cond_wait will never awake */
	if (signalled)
	{
		sum = check_waiting_nodes(fds, nfds, entries);
		if (sum) return sum;
	}

	/* STEP 4: At this point, no events have occured so we just have to wait for polling_thread_cv. */
	mutex_lock(&lock);
	if (!signalled)
	{
		cond_wait(&polling_thread_cv, &lock);
	}
	mutex_unlock(&lock);

	/* STEP 5: now you are awake, 
	go through all LL of every requested queue and count revents field of the ones that are set */
	sum = check_waiting_nodes(fds, nfds, entries);

	// return num of all queues that are signalled
	return sum;
}