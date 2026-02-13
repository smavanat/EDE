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
    pb_event_type type; //Type of the event
    void *data; //Data the event needs to transfter
} pb_event;

//The callback function called by publishing an event. *event is the event. *user_data represents the type calling the event
typedef void (*pb_event_callback)(const pb_event *event, void *user_data);

//Wrapper class for holding the data for the callback in a doubly linked list
typedef struct pb_subscriber{
    pb_event_callback callback; //Callback function
    void *user_data; //Data for the callback function
    struct pb_subscriber *next, *prev; //Next and previous elements in the doubly linked list
} pb_subscriber;

//Manager for all the linked lists for all of the event types
typedef struct {
    pb_subscriber *subscribers[MAX_EVENT_TYPES];
} pb_event_manager;

/**
 * Initialises the event manager for the pub-sub event pattern
 * @param manager: the event manager to be initialised
 */
void pb_event_manager_init(pb_event_manager *manager); //Initialises the event manager
/**
 * Adds a subscriber to an event publisher
 * @param manager: the manager managing the current event
 * @param type: the type of event
 * @param cb: the callback function to be called upon this event being published
 * @param user_data: the struct that want to be subscribed to this event
 */
void pb_event_manager_subscribe(pb_event_manager *manager, pb_event_type type, pb_event_callback cb, void *user_data); //Subscribes a struct to an event
/*
 * Removes a subscriber from an event publisher
 * @param manager: the manager managing the current event
 * @param type: the type of event
 * @param user_data: the struct that will unsubscribe from this event
 */
void pb_event_manager_unsubscribe(pb_event_manager *manager, pb_event_type type, void *user_data); //Unsubscribes a struct from an event
/*
 * Publishes an event from the event manager
 * @param manager: the event manager
 * @param the event to be published
 */
void pb_event_manager_publish(pb_event_manager *manager, pb_event *event); //Publishes an event
/*
 * Clears all of the subscribers from an event manager
 * @param manager: the event manager to be cleared
 */
void pb_event_manager_clear(pb_event_manager *manager); //Clears all the data stored in a manager;
#endif
