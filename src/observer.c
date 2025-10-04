#include "../include/observer.h"
#include <stdlib.h>

//Parameters:
//*subject: The subject to initialise
void subject_init(subject *subject) {
    subject->observers = list_alloc(INITIAL_CAPACITY, sizeof(observer));
}

//Parameters:
//*subject: The subject to free
void subject_free(subject *subject) {
    for(int i = 0; i < subject->observers->size; i++) {
        free(get_value(subject->observers, observer *, i));
    }
    free(subject->observers->data);
}

//Parameters:
//*subject: The subject to have an observer added
//cb: The callback function
//*context: The data of the object that wants to observe the subject
void subject_add_observer(subject *subject, ob_event_callback cb, void *context) {
    observer *ob = malloc(sizeof(observer));
    ob->cb = cb;
    ob->context = context;

    push_value(subject->observers, observer *, ob);
}

//Parameters:
//*subject: The subject to remove the observer from
//*context: The data of the object to remove
void subject_remove_observer(subject *subject, void *context) {
    for(int i = 0; i < subject->observers->size; i++) {
        if(get_value(subject->observers, observer *, i) ->context == context) {
            remove_at(subject->observers, observer *, i);
            return;
        }
    }
}

//Parameters:
//*subject: The subject to publish an event from
//*data: Any necessary event data
void subject_notify(subject *subject, void *data) {
    for(int i = 0; i < subject->observers->size; i++) {
        get_value(subject->observers, observer *, i)->cb(get_value(subject->observers, observer *, i)->context, data);
    }
}
