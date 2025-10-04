#ifndef __EVENT_H__
#define __EVENT_H__
//Need to make the implementation use an event queue rather than being synchronous. But that can be done later

//Event enum
typedef enum {
    EVENT_TYPE_COLLIDER,
    EVENT_TYPE_COUNT //End enum type for holding number of events
} event_type;

#define MAX_EVENT_TYPES EVENT_TYPE_COUNT

//Basic event struct. Holds the type of the event and any data that it needs to transfer
typedef struct {
    event_type type;
    void *data;
} event;

//The callback function called by publishing an event. *event is the event. *user_data represents the type calling the event
typedef void (*event_callback)(const event *event, void *user_data);

//Wrapper class for holding the data for the callback in a doubly linked list
typedef struct subscriber{
    event_callback callback;
    void *user_data;
    struct subscriber *next, *prev;
} subscriber;

//Manager for all the linked lists for all of the event types
typedef struct {
    subscriber *subscribers[MAX_EVENT_TYPES];
} event_manager;

void event_manager_init(event_manager *manager); //Initialises the event manager
void event_manager_subscribe(event_manager *manager, event_type type, event_callback cb, void *user_data); //Subscribes a struct to an event
void event_manager_unsubscribe(event_manager *manager, event_type type, void *user_data); //Unsubscribes a struct from an event
void event_manager_publish(event_manager *manager, event *event); //Publishes an event
void event_manager_clear(event_manager *manager); //Clears all the data stored in a manager;
#endif
