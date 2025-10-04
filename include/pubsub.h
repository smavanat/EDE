#ifndef __PUBSUB_H__
#define __PUBSUB_H__
//Implementation of the publisher/subscriber pattern
//Need to make the implementation use an event queue rather than being synchronous. But that can be done later

//Event enum
typedef enum {
    EVENT_TYPE_COLLIDER,
    EVENT_TYPE_COUNT //End enum type for holding number of events
} pb_event_type;

#define MAX_EVENT_TYPES EVENT_TYPE_COUNT

//Basic event struct. Holds the type of the event and any data that it needs to transfer
typedef struct {
    pb_event_type type;
    void *data;
} pb_event;

//The callback function called by publishing an event. *event is the event. *user_data represents the type calling the event
typedef void (*pb_event_callback)(const pb_event *event, void *user_data);

//Wrapper class for holding the data for the callback in a doubly linked list
typedef struct pb_subscriber{
    pb_event_callback callback;
    void *user_data;
    struct pb_subscriber *next, *prev;
} pb_subscriber;

//Manager for all the linked lists for all of the event types
typedef struct {
    pb_subscriber *subscribers[MAX_EVENT_TYPES];
} pb_event_manager;

void pb_event_manager_init(pb_event_manager *manager); //Initialises the event manager
void pb_event_manager_subscribe(pb_event_manager *manager, pb_event_type type, pb_event_callback cb, void *user_data); //Subscribes a struct to an event
void pb_event_manager_unsubscribe(pb_event_manager *manager, pb_event_type type, void *user_data); //Unsubscribes a struct from an event
void pb_event_manager_publish(pb_event_manager *manager, pb_event *event); //Publishes an event
void pb_event_manager_clear(pb_event_manager *manager); //Clears all the data stored in a manager;
#endif
