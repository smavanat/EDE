#include "../include/pubsub.h"
#include <stdlib.h>

//Parameters:
//*manager: the event manager to be initialised
void pb_event_manager_init(pb_event_manager *manager) {
    for(int i = 0; i < MAX_EVENT_TYPES; i++) {
        manager->subscribers[i] = NULL;
    }
}

//Parameters:
//*manager: the manager managing the current event
//type: the type of event
//cb: the callback function to be called upon this event being published
//*user_data: the struct that want to be subscribed to this event
void pb_event_manager_subscribe(pb_event_manager *manager, pb_event_type type, pb_event_callback cb, void *user_data) {
    pb_subscriber *new_subscriber = malloc(sizeof(pb_subscriber));
    new_subscriber->callback = cb;
    new_subscriber->user_data = user_data;
    new_subscriber->next = manager->subscribers[type];
    new_subscriber->prev = NULL;
    manager->subscribers[type]->prev = new_subscriber;
    manager->subscribers[type] = new_subscriber;
}

//Parameters:
//*manager: the manager managing the current event
//type: the type of event
//*user_data: the struct that will unsubscribe from this event
void pb_event_manager_unsubscribe(pb_event_manager *manager, pb_event_type type, void *user_data) {
    pb_subscriber *current = manager->subscribers[type];
    while(current->user_data != user_data && current != NULL) {
        current = current->next;
    }

    if(current != NULL) {
        current->prev->next = current->next;
        free(current);
    }
}

//Parameters:
//*manager: the event manager
//the event to be published
void pb_event_manager_publish(pb_event_manager *manager, pb_event *event) {
    pb_subscriber *current = manager->subscribers[event->type];
    while(current) {
        current->callback(event, current->user_data);
        current = current->next;
    }
}

//Parameters:
//*manager: the event manager to be cleared
void pb_event_manager_clear(pb_event_manager *manager) {
    for(int i = 0; i < MAX_EVENT_TYPES; i++) {
        pb_subscriber *current = manager->subscribers[i];
        while(current) {
            pb_subscriber *temp = current;
            current = current->next;
            free(temp);
        }
        manager->subscribers[i] = NULL;
    }
}
