#ifndef EZGL_CALLBACK_HPP
#define EZGL_CALLBACK_HPP

#include <ezgl/application.hpp>
#include <ezgl/camera.hpp>
#include <ezgl/canvas.hpp>
#include <ezgl/control.hpp>
#include <ezgl/graphics.hpp>
#include "../global.h"
#include "m1.h"
#include <iostream>
#include <string>
#include <vector>

namespace ezgl {
    
/**** Callback functions for keyboard and mouse input, and for all the ezgl predefined buttons. *****/

/**
 * React to a <a href = "https://developer.gnome.org/gtk3/stable/GtkWidget.html#GtkWidget-key-press-event">keyboard
 * press event</a>.
 *
 * @param widget The GUI widget where this event came from.
 * @param event The keyboard event.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_key(GtkWidget *widget, GdkEventKey *event, gpointer data);

/**
 * React to <a href = "https://developer.gnome.org/gtk3/stable/GtkWidget.html#GtkWidget-button-press-event">mouse click
 * event</a>
 *
 * @param widget The GUI widget where this event came from.
 * @param event The click event.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_mouse(GtkWidget *widget, GdkEventButton *event, gpointer data);

/**
 * React to <a href = "https://developer.gnome.org/gtk3/stable/GtkWidget.html#GtkWidget-button-release-event">mouse release
 * event</a>
 *
 * @param widget The GUI widget where this event came from.
 * @param event The click event.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean release_mouse(GtkWidget *widget, GdkEventButton *event, gpointer data);

/**
 * React to <a href = "https://developer.gnome.org/gtk3/stable/GtkWidget.html#GtkWidget-button-release-event">mouse release
 * event</a>
 *
 * @param widget The GUI widget where this event came from.
 * @param event The click event.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean move_mouse(GtkWidget *widget, GdkEventButton *event, gpointer data);

/**
 * React to <a href = "https://developer.gnome.org/gtk3/stable/GtkWidget.html#GtkWidget-scroll-event"> scroll_event
 * event</a>
 *
 * @param widget The GUI widget where this event came from.
 * @param event The click event.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean scroll_mouse(GtkWidget *widget, GdkEvent *event, gpointer data);

/**
 * React to the clicked zoom_fit button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_zoom_fit(GtkWidget *widget, gpointer data);

/**
 * React to the clicked zoom_in button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_zoom_in(GtkWidget *widget, gpointer data);

/**
 * React to the clicked zoom_out button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_zoom_out(GtkWidget *widget, gpointer data);

/**
 * React to the clicked up button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_up(GtkWidget *widget, gpointer data);

/**
 * React to the clicked up button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_down(GtkWidget *widget, gpointer data);

/**
 * React to the clicked up button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_left(GtkWidget *widget, gpointer data);

/**
 * React to the clicked up button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_right(GtkWidget *widget, gpointer data);

/**
 * React to the clicked proceed button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_proceed(GtkWidget *widget, gpointer data);

/**
 * React to the clicked find directions button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_help(GtkWidget *widget, gpointer data);

/**
 * React to the clicked find directions button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_find_directions(GtkWidget *widget, gpointer data);

/**
 * React to the clicked find button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_find(GtkWidget *widget, gpointer data);

/**
 * React to the clicked auto filter intersections button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_auto_intersections(GtkWidget *widget, gpointer data);

/**
 * React to the clicked auto filter streets button
 *
 * @param widget The GUI widget where this event came from.
 * @param data A pointer to any user-specified data you passed in.
 *
 * @return FALSE to allow other handlers to see this event, too. TRUE otherwise.
 */
gboolean press_auto_streets(GtkWidget *widget, gpointer data);

void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
//static void enter_callback( GtkWidget *, GtkWidget *entry );
}

#endif //EZGL_CALLBACK_HPP
