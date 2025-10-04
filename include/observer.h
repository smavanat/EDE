#ifndef __OBSERVER_H__
#define __OBSERVER_H__
#include "../include/list.h"
//Implementation of the observer pattern

#define INITIAL_CAPACITY 10

typedef void (*ob_event_callback)(void *context, void *user_data); //Callback function when observer event is called

typedef struct {
    ob_event_callback cb;
    void *context; //The data of the object doing the observing
} observer;

typedef struct {
    list *observers;
} subject;

void subject_init(subject *subject); //Initialises a subject
void subject_free(subject *subject); //Frees a subject
void subject_add_observer(subject *subject, ob_event_callback cb, void *context); //Adds an observer to a subject
void subject_remove_observer(subject *subject, void *context); //Removes an observer from a subject
void subject_notify(subject *subject, void *data); //Publishes an event from the subject to all observers

#endif
