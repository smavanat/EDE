#include "../include/event.h"
#include <stdlib.h>

void event_manager_init(event_manager *manager) {
    for(int i = 0; i < MAX_EVENT_TYPES; i++) {
        manager->subscribers[i] = NULL;
    }
}

void event_manager_subscribe(event_manager *manager, event_type type, event_callback cb, void *user_data) {
    subscriber *new_subscriber = malloc(sizeof(subscriber));
    new_subscriber->callback = cb;
    new_subscriber->user_data = user_data;
    new_subscriber->next = manager->subscribers[type];
    new_subscriber->prev = NULL;
    manager->subscribers[type]->prev = new_subscriber;
    manager->subscribers[type] = new_subscriber;
}

void event_manager_unsubscribe(event_manager *manager, event_type type, void *user_data) {
    subscriber *current = manager->subscribers[type];
    while(current->user_data != user_data && current != NULL) {
        current = current->next;
    }

    if(current != NULL) {
        current->prev->next = current->next;
        free(current);
    }
}
void event_manager_publish(event_manager *manager, event *event) {
    subscriber *current = manager->subscribers[event->type];
    while(current) {
        current->callback(event, current->user_data);
        current = current->next;
    }
}

void event_manager_clear(event_manager *manager) {
    for(int i = 0; i < MAX_EVENT_TYPES; i++) {
        subscriber *current = manager->subscribers[i];
        while(current) {
            subscriber *temp = current;
            current = current->next;
            free(temp);
        }
        manager->subscribers[i] = NULL;
    }
}
