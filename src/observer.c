#include "../include/observer.h"
#include <stdlib.h>

/**
 * Initialises the subject in an observer pattern
 * @param subject: The subject to initialise
 */
void subject_init(subject *subject) {
    subject->observers = list_alloc(INITIAL_CAPACITY, sizeof(observer));
}

/**
 * Frees the subject in an observer pattern
 * @param subject: The subject to free
 */
void subject_free(subject *subject) {
    for(int i = 0; i < subject->observers->size; i++) {
        free(get_value(subject->observers, observer *, i));
    }
    free(subject->observers->data);
}

/**
 * Adds an observer to the subject of the observer pattern
 * @param subject: The subject to have an observer added
 * @param cb: The callback function to be called on an event firing
 * @param context: The data of the object that wants to observe the subject
 */
void subject_add_observer(subject *subject, ob_event_callback cb, void *context) {
    observer *ob = malloc(sizeof(observer));
    ob->cb = cb;
    ob->context = context;

    push_value(subject->observers, observer *, ob);
}

/**
 * Removes an observer from a subject in the observer pattern
 * @param subject: The subject to remove the observer from
 * @param context: The data of the object to remove
 */
void subject_remove_observer(subject *subject, void *context) {
    for(int i = 0; i < subject->observers->size; i++) {
        if(get_value(subject->observers, observer *, i) ->context == context) {
            remove_at(subject->observers, observer *, i);
            return;
        }
    }
}

/**
 * Notifies all of the observers of a subject on the firing of an event by calling their callback functions
 * @param subject: The subject to publish an event from
 * @param data: Any necessary event data
 */
void subject_notify(subject *subject, void *data) {
    for(int i = 0; i < subject->observers->size; i++) {
        get_value(subject->observers, observer *, i)->cb(get_value(subject->observers, observer *, i)->context, data);
    }
}
