#ifndef __EVENT_H__
#define __EVENT_H__

typedef enum {
    EVENT_TYPE_COUNT
} event_type;

#define MAX_EVENT_TYPES EVENT_TYPE_COUNT

typedef struct {
    event_type type;
    void *data;
} event;

typedef void (*event_callback)(const event *event, void *user_data);

typedef struct subscriber{
    event_callback callback;
    void *user_data;
    struct subscriber *next, *prev;
} subscriber;

typedef struct {
    subscriber *subscribers[MAX_EVENT_TYPES];
} event_manager;

void event_manager_init(event_manager *manager);
void event_manager_subscribe(event_manager *manager, event_type type, event_callback cb, void *user_data);
void event_manager_unsubscribe(event_manager *manager, event_type type, void *user_data);
void event_manager_publish(event_manager *manager, event *event);
void event_manager_clear(event_manager *manager);
#endif
