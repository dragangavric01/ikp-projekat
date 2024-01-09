#include "topic_queue.h"
#include "subscriber_list.h"

typedef struct TopicStruct {
	char* name;
	TopicQueuePointers queue_pointers;
	SubscriberList subscriber_list;
} Topic;

