#ifndef __OBSERVER_H__
#define __OBSERVER_H__
#include "../include/list.h"
//Implementation of the observer pattern

#define INITIAL_CAPACITY 10

typedef void (*ob_event_callback)(void *context, void *user_data); //Callback function when observer event is called

typedef struct {
    ob_event_callback cb; //The callback function of the observer to be called when an event fires
    void *context; //The data of the object doing the observing
} observer;

typedef struct {
    list *observers;
} subject;

/**
 * Initialises the subject in an observer pattern
 * @param subject: The subject to initialise
 */
void subject_init(subject *subject);
/**
 * Frees the subject in an observer pattern
 * @param subject: The subject to free
 */
void subject_free(subject *subject);
/**
 * Adds an observer to the subject of the observer pattern
 * @param subject: The subject to have an observer added
 * @param cb: The callback function to be called on an event firing
 * @param context: The data of the object that wants to observe the subject
 */
void subject_add_observer(subject *subject, ob_event_callback cb, void *context);
/**
 * Removes an observer from a subject in the observer pattern
 * @param subject: The subject to remove the observer from
 * @param context: The data of the object to remove
 */
void subject_remove_observer(subject *subject, void *context);
/**
 * Notifies all of the observers of a subject on the firing of an event by calling their callback functions
 * @param subject: The subject to publish an event from
 * @param data: Any necessary event data
 */
void subject_notify(subject *subject, void *data);

#endif
