#include "ezgl/callback.hpp"
#include "m1.cpp"
#include "m2.cpp"
#include <iostream>

namespace ezgl {
    
// File wide static variables to track whether the middle mouse
// button is currently pressed AND the old x and y positions of the mouse pointer
bool middle_mouse_button_pressed = false;
int last_panning_event_time = 0;
double prev_x = 0, prev_y = 0;

gboolean press_key(GtkWidget *, GdkEventKey *event, gpointer data)
{
    auto application = static_cast<ezgl::application *>(data);

    // Call the user-defined key press callback if defined
    if(application->key_press_callback != nullptr) {
      // see: https://developer.gnome.org/gdk3/stable/gdk3-Keyboard-Handling.html
      application->key_press_callback(application, event, gdk_keyval_name(event->keyval));
    }

    // TODO
    GtkEntryCompletion* autocomplete1 = gtk_entry_completion_new();
    GtkEntryCompletion* autocomplete2 = gtk_entry_completion_new();

    // Get the GtkEntry object
    GtkEntry* text_entry1 = (GtkEntry *) application->get_object("FindStreet1");
    GtkEntry* text_entry2 = (GtkEntry *) application->get_object("FindStreet2");

    // Get the text written in the widget
    std::string text1 = gtk_entry_get_text(text_entry1);
    std::string text2 = gtk_entry_get_text(text_entry2);
    
    // Drop down lists and iterators (this one is for street 1))
    GtkListStore* list1 = gtk_list_store_new(1,G_TYPE_STRING);
    GtkTreeIter iter1;
    // Drop down for street 2
    GtkListStore* list2 = gtk_list_store_new(1,G_TYPE_STRING);
    GtkTreeIter iter2;
    
    int display_size;
    std::string item_name;

    // If the user input is greater than 1 character, drop down suggestions
    if(text1.length() > 1){
        std::vector<unsigned> item_ids;
        
        if(!filter_streets && filter_intersections)
            item_ids = find_street_ids_from_partial_street_name(text1);
        else if(!filter_intersections && filter_streets)
            item_ids = find_intersection_ids_from_partial_intersection_name(text1);
        else if(!filter_streets && !filter_intersections && (text1.find('&') != std::string::npos || text1.find(',') != std::string::npos || text1.find(" and ") != std::string::npos))
            item_ids = find_intersection_ids_from_partial_intersection_name(text1);
        else if(!filter_streets && !filter_intersections)
            item_ids = find_street_ids_from_partial_street_name(text1);
        
        if(item_ids.size() < 6 || (filter_streets && !filter_intersections))
            display_size = item_ids.size();
        else
            display_size = 6;
        
        for(int i=0; i < display_size; i++){
            gtk_list_store_append(list1, &iter1);
            
            if(!filter_streets && filter_intersections)
                item_name = getStreetName(item_ids[i]);
            else if(!filter_intersections && filter_streets)
                item_name = getIntersectionName(item_ids[i]);
            else if(!filter_streets && !filter_intersections && (text1.find('&') != std::string::npos || text1.find(',') != std::string::npos || text1.find(" and ") != std::string::npos))
                item_name = getIntersectionName(item_ids[i]);
            else if(!filter_streets && !filter_intersections)
                item_name = getStreetName(item_ids[i]);
            
            gchar *gtkstring = new gchar[item_name.length()+1];
            for(size_t j=0; j <= item_name.length(); j++)
                gtkstring[j] = item_name[j];
            
            gtk_list_store_set(list1, &iter1, 0, gtkstring, -1);
            g_free(gtkstring);
            
        }
    }
    
    gtk_entry_completion_set_model(autocomplete1, GTK_TREE_MODEL(list1));
    gtk_entry_set_completion(GTK_ENTRY(text_entry1), autocomplete1);
    gtk_entry_completion_set_text_column(autocomplete1, 0);
    
    
    // If the user input is greater than 1 character, drop down suggestions
    if(text2.length() > 1){
        std::vector<unsigned> item_ids;
        
        if(!filter_streets && filter_intersections)
            item_ids = find_street_ids_from_partial_street_name(text2);
        else if(!filter_intersections && filter_streets)
            item_ids = find_intersection_ids_from_partial_intersection_name(text2);
        else if(!filter_streets && !filter_intersections && (text2.find('&') != std::string::npos || text2.find(',') != std::string::npos || text2.find(" and ") != std::string::npos))
            item_ids = find_intersection_ids_from_partial_intersection_name(text2);
        else if(!filter_streets && !filter_intersections)
            item_ids = find_street_ids_from_partial_street_name(text2);
        
        if(item_ids.size() < 6 || (filter_streets && !filter_intersections))
            display_size = item_ids.size();
        else
            display_size = 6;
        
        for(int i=0; i < display_size; i++){
            gtk_list_store_append(list2, &iter2);
            
            if(!filter_streets && filter_intersections)
                item_name = getStreetName(item_ids[i]);
            else if(!filter_intersections && filter_streets)
                item_name = getIntersectionName(item_ids[i]);
            else if(!filter_streets && !filter_intersections && (text2.find('&') != std::string::npos || text2.find(',') != std::string::npos || text2.find(" and ") != std::string::npos))
                item_name = getIntersectionName(item_ids[i]);
            else if(!filter_streets && !filter_intersections)
                item_name = getStreetName(item_ids[i]);
            
            gchar *gtkstring = new gchar[item_name.length()+1];
            for(size_t j=0; j <= item_name.length(); j++)
                gtkstring[j] = item_name[j];
            
            gtk_list_store_set(list2, &iter2, 0, gtkstring, -1);
            g_free(gtkstring);
            
        }
    }
    
    gtk_entry_completion_set_model(autocomplete2, GTK_TREE_MODEL(list2));
    gtk_entry_set_completion(GTK_ENTRY(text_entry2), autocomplete2);
    gtk_entry_completion_set_text_column(autocomplete2, 0);
    
    return FALSE; // propagate the event
}

gboolean press_mouse(GtkWidget *, GdkEventButton *event, gpointer data)
{
  auto application = static_cast<ezgl::application *>(data);

  if(event->type == GDK_BUTTON_PRESS) {

    // Check for Middle mouse press to support dragging
    if(event->button == 2) {
      middle_mouse_button_pressed = true;
      prev_x = event->x;
      prev_y = event->y;
    }

    // Call the user-defined mouse press callback if defined
    if(application->mouse_press_callback != nullptr) {
      ezgl::point2d const widget_coordinates(event->x, event->y);

      std::string main_canvas_id = application->get_main_canvas_id();
      ezgl::canvas *canvas = application->get_canvas(main_canvas_id);

      ezgl::point2d const world = canvas->get_camera().widget_to_world(widget_coordinates);
      application->mouse_press_callback(application, event, world.x, world.y);
    }
  }

  return TRUE; // consume the event
}

gboolean release_mouse(GtkWidget *, GdkEventButton *event, gpointer )
{
  if(event->type == GDK_BUTTON_RELEASE) {
    // Check for Middle mouse release to support dragging
    if(event->button == 2) {
      middle_mouse_button_pressed = false;
    }
  }

  return TRUE; // consume the event
}

gboolean move_mouse(GtkWidget *, GdkEventButton *event, gpointer data)
{
  auto application = static_cast<ezgl::application *>(data);

  if(event->type == GDK_MOTION_NOTIFY) {

    // Check if the middle mouse is pressed to support dragging
    if(middle_mouse_button_pressed) {
      // drop this panning event if we have just served another one
      if(gtk_get_current_event_time() - last_panning_event_time < 100)
        return true;

      last_panning_event_time = gtk_get_current_event_time();

      GdkEventMotion *motion_event = (GdkEventMotion *)event;

      std::string main_canvas_id = application->get_main_canvas_id();
      auto canvas = application->get_canvas(main_canvas_id);

      point2d curr_trans = canvas->get_camera().widget_to_world({motion_event->x, motion_event->y});
      point2d prev_trans = canvas->get_camera().widget_to_world({prev_x, prev_y});

      double dx = curr_trans.x - prev_trans.x;
      double dy = curr_trans.y - prev_trans.y;

      prev_x = motion_event->x;
      prev_y = motion_event->y;

      // Flip the delta x to avoid inverted dragging
      translate(canvas, -dx, -dy);
    }
    // Else call the user-defined mouse move callback if defined
    else if(application->mouse_move_callback != nullptr) {
      ezgl::point2d const widget_coordinates(event->x, event->y);

      std::string main_canvas_id = application->get_main_canvas_id();
      ezgl::canvas *canvas = application->get_canvas(main_canvas_id);

      ezgl::point2d const world = canvas->get_camera().widget_to_world(widget_coordinates);
      application->mouse_move_callback(application, event, world.x, world.y);
    }
  }

  return TRUE; // consume the event
}

gboolean scroll_mouse(GtkWidget *, GdkEvent *event, gpointer data)
{

  if(event->type == GDK_SCROLL) {
    auto application = static_cast<ezgl::application *>(data);

    std::string main_canvas_id = application->get_main_canvas_id();
    auto canvas = application->get_canvas(main_canvas_id);

    GdkEventScroll *scroll_event = (GdkEventScroll *)event;

    ezgl::point2d scroll_point(scroll_event->x, scroll_event->y);

    if(!zoom_level_end && scroll_event->direction == GDK_SCROLL_UP) {
      // Zoom in at the scroll point
        ezgl::zoom_in(canvas, scroll_point, 5.0 / 3.0);
    } else if(scroll_event->direction == GDK_SCROLL_DOWN) {
      // Zoom out at the scroll point
      ezgl::zoom_out(canvas, scroll_point, 5.0 / 3.0);
    } else if(scroll_event->direction == GDK_SCROLL_SMOOTH) {
      // Doesn't seem to be happening
    } // NOTE: We ignore scroll GDK_SCROLL_LEFT and GDK_SCROLL_RIGHT
  }
  return TRUE;
}

gboolean press_zoom_fit(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::zoom_fit(canvas, canvas->get_camera().get_initial_world());

  return TRUE;
}

gboolean press_zoom_in(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  if(!zoom_level_end)
    ezgl::zoom_in(canvas, 5.0 / 3.0);

  return TRUE;
}

gboolean press_zoom_out(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::zoom_out(canvas, 5.0 / 3.0);

  return TRUE;
}

gboolean press_up(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::translate_up(canvas, 5.0);

  return TRUE;
}

gboolean press_down(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::translate_down(canvas, 5.0);

  return TRUE;
}

gboolean press_left(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::translate_left(canvas, 5.0);

  return TRUE;
}

gboolean press_right(GtkWidget *, gpointer data)
{

  auto application = static_cast<ezgl::application *>(data);

  std::string main_canvas_id = application->get_main_canvas_id();
  auto canvas = application->get_canvas(main_canvas_id);

  ezgl::translate_right(canvas, 5.0);

  return TRUE;
}

gboolean press_proceed(GtkWidget *, gpointer data)
{
  auto ezgl_app = static_cast<ezgl::application *>(data);
  ezgl_app->quit();
  
  return TRUE;
}

void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer)
{
    // For demonstration purposes, this will show the int value of the response type
    std::cout << "response is ";
    
    switch(response_id) {
        
        case GTK_RESPONSE_ACCEPT:
            std::cout << "GTK_RESPONSE_ACCEPT ";
        break;
        case GTK_RESPONSE_DELETE_EVENT:
            std::cout << "GTK_RESPONSE_DELETE_EVENT (i.e. ’X’ button) ";
        break;
        case GTK_RESPONSE_REJECT:
            std::cout << "GTK_RESPONSE_REJECT ";
        break;
        default:
            std::cout << "UNKNOWN ";
        break;
    }
    
    std::cout << "(" << response_id << ")\n";
    /*This will cause the dialog to be destroyed*/
    gtk_widget_destroy(GTK_WIDGET (dialog));
}


gboolean press_find_directions(GtkWidget *, gpointer data){
    start_id = NO_VALUE;
    
    // BEGIN: CODE FOR SHOWING DIALOG
    GObject   *window; 
    GtkWidget *new_window;         // the parent window over which to add the dialog
    GtkWidget *scrolled_window;
    GtkWidget *content_area; // the content area of the dialog (i.e. where to put stuff in the dialog)
    GtkWidget *label; // the label we will create to display a message in the content area
    GtkWidget *dialog; // the dialog box we will create
    
    // Turn off previous highlights (this may seem really slow)
    //
    for(unsigned i = 0; i < intersections.size(); i++)
        intersections[i].highlight = false;
    for(unsigned i = 0; i < street_segments.size(); i++)
        street_segments[i].highlight = false;
    
    distanceFinder = true;
    
    auto application = static_cast<ezgl::application *>(data);
    
    // get a pointer to the main application window
    window = application->get_object(application->get_main_window_id().c_str());
    
    // Get the GtkEntry object
    GtkEntry* text_entry1 = (GtkEntry *) application->get_object("FindStreet1");
    GtkEntry* text_entry2 = (GtkEntry *) application->get_object("FindStreet2");

    
    // Get the text written in the widget
    std::string text1 = gtk_entry_get_text(text_entry1);
    std::string text2 = gtk_entry_get_text(text_entry2);
    
    std::vector<unsigned> intersection1 = find_intersection_ids_from_partial_intersection_name(text1);
    std::vector<unsigned> intersection2 = find_intersection_ids_from_partial_intersection_name(text2);
    
    
    
    if(text2.length() == 0 || text1.length() == 0){
        // Case for searching for a street. iterate through all streets given text1 as a prefix
        
        //Provides meaningful error messages when user input is invalid
        if(intersection2.size() == 0 || intersection1.size() == 0){
            dialog = gtk_dialog_new_with_buttons("ERROR", (GtkWindow*) window, GTK_DIALOG_MODAL, ("OK"), GTK_RESPONSE_ACCEPT, NULL, GTK_RESPONSE_REJECT, NULL);
            
            // Create a label and attach it to the content area of the dialog
            content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
            label = gtk_label_new("Please Enter an Intersection!");
            gtk_container_add(GTK_CONTAINER(content_area), label);
            
            // The main purpose of this is to show dialog’s child widget, label
            gtk_widget_show_all(dialog);
            
            // Connecting the "response" signal from the user to the associated callback function
            g_signal_connect(GTK_DIALOG(dialog), "response", G_CALLBACK(on_dialog_response), NULL);
        }
    }else if(text2.length() > 0 && text1.length() > 0){
        
        if(intersection1.size() == 0 || intersection2.size() == 0){
            dialog = gtk_dialog_new_with_buttons("ERROR", (GtkWindow*) window, GTK_DIALOG_MODAL, ("OK"), GTK_RESPONSE_ACCEPT, NULL, GTK_RESPONSE_REJECT, NULL);
            
            // Create a label and attach it to the content area of the dialog
            content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
            label = gtk_label_new("Intersection Does Not Exist!");
            gtk_container_add(GTK_CONTAINER(content_area), label);
            
            // The main purpose of this is to show dialog’s child widget, label
            gtk_widget_show_all(dialog);
            
            // Connecting the "response" signal from the user to the associated callback function
            g_signal_connect(GTK_DIALOG(dialog), "response", G_CALLBACK(on_dialog_response), NULL);  

        }else if(intersection1.size() > 0 && intersection2.size() > 0){
            intersections[intersection1[0]].highlight = true;
            intersections[intersection2[0]].highlight = true;
            
            start_id = intersection1[0];
            
            std::vector<unsigned> directions_path = find_path_between_intersections(intersection1[0], intersection2[0], 15, 25);
            int travel_time = compute_path_travel_time(directions_path, 15, 25)/60;
            
            if(directions_path.size() == 0){
                dialog = gtk_dialog_new_with_buttons("ERROR", (GtkWindow*) window, GTK_DIALOG_MODAL, ("OK"), GTK_RESPONSE_ACCEPT, NULL, GTK_RESPONSE_REJECT, NULL);
                
                // Create a label and attach it to the content area of the dialog
                content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
                label = gtk_label_new("No Path is Found!");
                gtk_container_add(GTK_CONTAINER(content_area), label);
                
                // The main purpose of this is to show dialog’s child widget, label
                gtk_widget_show_all(dialog);
                
                // Connecting the "response" signal from the user to the associated callback function
                g_signal_connect(GTK_DIALOG(dialog), "response", G_CALLBACK(on_dialog_response), NULL);
            }
            //looping through all the street segments and highlighting the ones that correspond to those in the path directions vector
            for(unsigned i = 0; i < street_segments.size(); i++){
                for(unsigned j = 0; j < directions_path.size(); j++){
                    unsigned temp_start = getInfoStreetSegment(directions_path[j]).from;
                    unsigned temp_end = getInfoStreetSegment(directions_path[j]).to;
                    LatLon start_pos1 = getIntersectionPosition(temp_start);
                    LatLon end_pos1 = getIntersectionPosition(temp_end);
                    LatLon start_pos2 = street_segments[i].start_position;
                    LatLon end_pos2 = street_segments[i].end_position;
                    float start_x1 = x_from_lon(start_pos1.lon());
                    float start_y1 = y_from_lat(start_pos1.lat());
                    float end_x1 = x_from_lon(end_pos1.lon());
                    float end_y1 = y_from_lat(end_pos1.lat());
                    float start_x2 = x_from_lon(start_pos2.lon());
                    float start_y2 = y_from_lat(start_pos2.lat());
                    float end_x2 = x_from_lon(end_pos2.lon());
                    float end_y2 = y_from_lat(end_pos2.lat());
                    //comparing the starting and ending points of the street segments to check if it is the same segment
                    if((start_x1 == start_x2 && end_x1 == end_x2) && (start_y1 == start_y2 && end_y1 == end_y2)){
                            street_segments[i].highlight = true;
                    }
                }        
            }
            
            int current_street_segment_length;
            unsigned current_id;
            std::string travel_directions;

            //looping through the path directions and adding to the string that stores all the directions to be printed
            for(unsigned k = 0; k < directions_path.size() - 1; k++){
                std::string turn;
                bool next_turn_exists = false;
                TurnType next_turn = TurnType::NONE;
                TurnType temp = find_turn_type(directions_path[k], directions_path[k + 1]);

                if(k < directions_path.size() - 2){
                    next_turn = find_turn_type(directions_path[k + 1], directions_path[k + 2]);
                    next_turn_exists = true;
                }
                if(temp == TurnType::LEFT){
                    turn = "left";
                }
                else if(temp == TurnType::RIGHT){
                    turn = "right";
                }
                else if(temp == TurnType::STRAIGHT){
                    turn = "straight";
                }

                //getting the values for the current street segment and the street segment to turn onto
                InfoStreetSegment current = getInfoStreetSegment(directions_path[k]);
                InfoStreetSegment next = getInfoStreetSegment(directions_path[k + 1]);
                current_id = current.streetID;
                unsigned next_id = next.streetID;
                std::string current_street = getStreetName(current_id);
                std::string next_street = getStreetName(next_id);
                current_street_segment_length = std::round(find_street_segment_length(directions_path[k]));
                int next_street_segment_length = std::round(find_street_segment_length(directions_path[k + 1]));

                if(turn == "straight"){ 
                    if(next_turn == TurnType::STRAIGHT && next_turn_exists){    
                        current_street_segment_length += std::round(find_street_segment_length(directions_path[k + 1]));                   
                    }else{
                        travel_directions += "Continue straight on "; 
                        travel_directions += current_street; 
                        travel_directions += " for "; 
                        travel_directions += std::to_string(static_cast<int>(current_street_segment_length)); 
                        travel_directions += " m \n\n";
                    }
                }
                else{
                   travel_directions += "Turn "; 
                   travel_directions += turn ;
                   travel_directions +=  " onto "; 
                   travel_directions += next_street; 
                   travel_directions += " in "; 
                   travel_directions += std::to_string(static_cast<int>(next_street_segment_length)); 
                   travel_directions += " m \n\n";
                }
            }

            travel_directions +=  "The total travel time between the intersections is "; 
            travel_directions += std::to_string(travel_time); 
            travel_directions += " minutes \n";
            
            new_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
            gtk_window_set_title (GTK_WINDOW (new_window), "Travel Directions");
            gtk_window_set_default_size (GTK_WINDOW (new_window), 520, 400);
            
            scrolled_window = gtk_scrolled_window_new (NULL, NULL);
            /* Set the border width */
            gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 10);
            /* Set the policy of the horizontal and vertical scrollbars to automatic.
            * What this means is that the scrollbars are only present if needed.
            */
            gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                            GTK_POLICY_AUTOMATIC,
                                            GTK_POLICY_AUTOMATIC);

            // Create a label and attach it to the content area of the dialog
            label = gtk_label_new(&travel_directions[0]);
            gtk_container_add (GTK_CONTAINER (new_window), scrolled_window);
            gtk_container_add (GTK_CONTAINER (scrolled_window), label);


            // Connecting the "response" signal from the user to the associated callback function
            g_signal_connect (G_OBJECT (label), "clicked",
                              G_CALLBACK (gtk_main_quit), NULL);
            
            /* set the window as visible */
            gtk_widget_show_all (new_window);
            
            intersection1.clear();
            intersection2.clear();
            
            gtk_entry_set_text(GTK_ENTRY(text_entry1), "");
            gtk_entry_set_text(GTK_ENTRY(text_entry2), "");
        }
    }
    else
        return TRUE;
    // Redraw the graphics
    application->refresh_drawing();
    
    return TRUE;
}

gboolean press_help(GtkWidget *, gpointer data){
    GtkWidget *window = NULL;         // the parent window over which to add the dialog
    GtkWidget *scrolled_window = NULL;
    GtkWidget *label = NULL;        // the label we will create to display a message in the content area
    
    auto application = static_cast<ezgl::application *>(data);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Help");
    gtk_window_set_default_size (GTK_WINDOW (window), 650, 750);

    // Create a label and attach it to the content area of the dialog
    //content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new("Choosing a Map: \n\n"
    "There are 19 different maps from across the world that can \n"
    "be chosen by selecting the corresponding number from the terminal. \n"
    "To move from one map to another, press “Quit”, minimize the map window, \n" 
    "and enter the new city number in the terminal. \n" "\n"
            
    "Navigating in the Map: \n\n"
    "Use the arrow keys to move up/down and left/right in the map. \n"
    "Zoom in or out using the “+” and “-” buttons or by scrolling in or out. \n" 
    "Depending on how zoomed out the user is, varying levels of detail will be \n"
    "displayed. Major roads are drawn in yellow, and minor roads in white. Parks and \n"
    "green spaces are shown in green. Lakes and rivers are shown in blue. Buildings are \n"
    "shown in dark grey."
            
    "Displaying Points of Interest and One-Way Streets: \n\n"
    "Press the “Toggle POIs” or “Toggle One-Ways” button to display points of \n"
    "interest and one way streets on the map. Click the same buttons again to \n"
    "temporarily remove them from the map. \n" "\n"
            
    "Finding the Nearest Intersection and Point of Interest: \n\n"
    "Press the “Nearest Inter” or “Nearest POI” buttons, and then click on any \n"
    "point on the map. The nearest intersection or point of interest will be \n"
    "highlighted and the name displayed in the bottom left corner of the screen. \n"
    "To use this feature again, click on the same button again. \n \n"
            
    "Finding Streets and Intersections by Name: \n\n"
    "To use the Find feature, enter one or two street names in the text boxes \n"
    "at the top of the screen and then click on the “Find” button. If one street \n" 
    "is entered, then the street will be highlighted on the map. If two street names \n" 
    "are entered, then the intersections between the two streets will be highlighted. \n"
    "The text boxes accept partial street names and give suggestions if the “Streets” \n"
    "box is checked \n\n"
    
    "Finding Directions between Intersections: \n\n"
    "To get directions between intersections, the names of the intersections can be \n"
    "entered into the text boxes at the top of the screen, or two points can be clicked \n"
    "on the screen, and the nearest intersections to the points will be selected and \n"
    "displayed in the text boxes and highlighted on the screen. After entering or clicking \n"
    "on the intersections, click on the “Directions” button and the path will be highlighted \n"
    "in red on the screen and the specific travel directions displayed in a new pop-up window. \n"
    "To access the map, minimize the window. The text boxes accept partial intersections names\n"
    "and give suggestions if the “Intersections” box is checked. To get directions again, click \n"
    "on the “Directions” button twice and repeat the process \n\n");
   /* Create the scrolled window. Usually NULL is passed for both parameters so
    * that it creates the horizontal/vertical adjustments automatically. Setting
    * the scrollbar policy to automatic allows the scrollbars to only show up
    * when needed.
    */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
   /* Set the border width */
    gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 10);
   /* Set the policy of the horizontal and vertical scrollbars to automatic.
    * What this means is that the scrollbars are only present if needed.
    */
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

    gtk_container_add (GTK_CONTAINER (window), scrolled_window);
    gtk_container_add (GTK_CONTAINER (scrolled_window), label);

    g_signal_connect (G_OBJECT (label), "clicked",
                    G_CALLBACK (gtk_main_quit), NULL);

    /* set the window as visible */
    gtk_widget_show_all (window);
    
    // Connecting the "response" signal from the user to the associated callback function
    //g_signal_connect(GTK_DIALOG(dialog), "response", G_CALLBACK(on_dialog_response), NULL);
    
    application->refresh_drawing();
    
    return TRUE;
}

gboolean press_find(GtkWidget *, gpointer data){
    start_id = NO_VALUE;
    // BEGIN: CODE FOR SHOWING DIALOG
    GObject *window = NULL; // the parent window over which to add the dialog
    GtkWidget *content_area = NULL; // the content area of the dialog (i.e. where to put stuff in the dialog)
    GtkWidget *label = NULL; // the label we will create to display a message in the content area
    GtkWidget *dialog = NULL; // the dialog box we will create

    // Turn off previous highlights (this may seem really slow)
    //
    for(unsigned i = 0; i < intersections.size(); i++)
        intersections[i].highlight = false;
    for(unsigned i = 0; i < street_segments.size(); i++)
        street_segments[i].highlight = false;
    
    auto application = static_cast<ezgl::application *>(data);
    
    // get a pointer to the main application window
    window = application->get_object(application->get_main_window_id().c_str());
    
    // Get the GtkEntry object
    GtkEntry* text_entry1 = (GtkEntry *) application->get_object("FindStreet1");
    GtkEntry* text_entry2 = (GtkEntry *) application->get_object("FindStreet2");
//    GtkEntry* text_entry3 = (GtkEntry *) application->get_object("FindStreet1");
    
    // Get the text written in the widget
    std::string text1 = gtk_entry_get_text(text_entry1);
    std::string text2 = gtk_entry_get_text(text_entry2);
//    std::string text3 = gtk_entry_get_text(text_entry2);
//    text3 = toLowerCase(text3);
    
    std::vector<unsigned> streets1 = find_street_ids_from_partial_street_name(text1);
    std::vector<unsigned> streets2 = find_street_ids_from_partial_street_name(text2);
//    std::vector<std::string> POI;
    
//    for(unsigned i = 0; i < POIs.size(); i++){
//        if(POIs[i].name == text3){
//            POI.push_back(POIs[i].name);
//        }
//    }
    
    
    
    // Turn on highlighting for focused street segments or intersections
    //
    if(text2.length() == 0 && text1.length() == 0){
        // Path finding testing because UI is big bad :((
//        const unsigned inter_start = 702;
//        const unsigned inter_end    = 981;
//        std::vector<unsigned> path = find_path_between_intersections(inter_start, inter_end, 0, 0);
//        
//        intersections[inter_start].highlight = true;
//        intersections[inter_end].highlight = true;
//    
//        std::cout << "Path size: " << path.size() << std::endl;
//
//        for(int i = 0; i < path.size(); i++){
//            std::cout << path[i] << std::endl;
//            street_segments[path[i]].highlight = true;
//        }
        
        dialog = gtk_dialog_new_with_buttons(
            "ERROR",
            (GtkWindow*) window,
            GTK_DIALOG_MODAL,
            ("OK"),
            GTK_RESPONSE_ACCEPT,
            NULL,
            GTK_RESPONSE_REJECT,
            NULL
        );
        // Create a label and attach it to the content area of the dialog
        content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        label = gtk_label_new("Please Enter Street Names!");
        gtk_container_add(GTK_CONTAINER(content_area), label);
        // The main purpose of this is to show dialog’s child widget, label
        gtk_widget_show_all(dialog);
        // Connecting the "response" signal from the user to the associated callback function
        g_signal_connect(
            GTK_DIALOG(dialog),
            "response",
            G_CALLBACK(on_dialog_response),
            NULL
        );
        
        
    }else if(text2.length() == 0 && text1.length() > 0){
        // Case for searching for a street. iterate through all streets given text1 as a prefix
        
        //Provides meaningful error messages when user input is invalid
        if(streets1.size() == 0){
            dialog = gtk_dialog_new_with_buttons(
            "ERROR",
            (GtkWindow*) window,
            GTK_DIALOG_MODAL,
            ("OK"),
            GTK_RESPONSE_ACCEPT,
            NULL,
            GTK_RESPONSE_REJECT,
            NULL
            );
            // Create a label and attach it to the content area of the dialog
            content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
            label = gtk_label_new("Invalid Street Name!");
            gtk_container_add(GTK_CONTAINER(content_area), label);
            // The main purpose of this is to show dialog’s child widget, label
            gtk_widget_show_all(dialog);
            // Connecting the "response" signal from the user to the associated callback function
            g_signal_connect(
                GTK_DIALOG(dialog),
                "response",
                G_CALLBACK(on_dialog_response),
                NULL
            );
        }
        
        for(unsigned i = 0; i < streets1.size(); i++){
            // get street segs if input text length is acceptable. Leading and trailing whitespace is counted right now
            if(text1.length() >= 2 || text1.length() >= getStreetName(streets1[i]).length()/2){
                // 
                std::vector<unsigned> select_segs = find_street_street_segments(streets1[i]);

                for(unsigned j = 0; j < select_segs.size(); j++)
                    street_segments[select_segs[j]].highlight = true;
            }
        }

    }else if(text2.length() > 0 && text1.length() > 0){
        // Case for searching for an intersection. iterate through all possible streets given text1 and text2 as prefixes
        
        //Provides meaningful error messages when user input is invalid
        if(streets1.size() == 0 || streets2.size() == 0){
            dialog = gtk_dialog_new_with_buttons(
            "ERROR",
            (GtkWindow*) window,
            GTK_DIALOG_MODAL,
            ("OK"),
            GTK_RESPONSE_ACCEPT,
            NULL,
            GTK_RESPONSE_REJECT,
            NULL
            );
            // Create a label and attach it to the content area of the dialog
            content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
            if(streets1.size() == 0 && streets2.size() == 0)
                label = gtk_label_new("Both Street Names are Invalid!");
            else if (streets2.size() == 0)
                label = gtk_label_new("Second Street Name is Invalid!");
            else if(streets1.size() == 0)
                label = gtk_label_new("First Street Name is Invalid!");
            gtk_container_add(GTK_CONTAINER(content_area), label);
            // The main purpose of this is to show dialog’s child widget, label
            gtk_widget_show_all(dialog);
            // Connecting the "response" signal from the user to the associated callback function
            g_signal_connect(
                GTK_DIALOG(dialog),
                "response",
                G_CALLBACK(on_dialog_response),
                NULL
            );
        }
        
        for(unsigned i = 0; i < streets1.size(); i++){
            for(unsigned j = 0; j < streets2.size(); j++){
                // Get street intersections if user input text1 and text2 meet the length requirements (Spaces are especially not wanted)
                if(     (text1.length() >= 2 || text1.length() >= getStreetName(streets1[i]).length()/2)
                        && (text2.length() >= 2 || text2.length() >= getStreetName(streets2[j]).length()/2)    )
                {
                    std::vector<unsigned> select_intersections = find_intersection_ids_from_street_ids(streets1[i], streets2[j]);

                    for(unsigned k = 0; k < select_intersections.size(); k++){
                        intersections[select_intersections[k]].highlight = true;
                    }
                }
            }
        }
    } else
        return TRUE;
        
    // Redraw the graphics
    application->refresh_drawing();
    
    return TRUE;
}

// On check box click, set filter_intersections to appropriate value
gboolean press_auto_intersections(GtkWidget *, gpointer data){
    
    auto application = static_cast<ezgl::application *>(data);
    
    // get check box for intersections
    auto check_inter = (GtkCheckButton *)application->get_object("AutoIntersections");
    // if its activated then don't filter intersections in auto-complete
    filter_intersections = !gtk_toggle_button_get_active(&(check_inter->toggle_button));
    
    return TRUE;
}

// On check box click, set filter_streets to appropriate value
gboolean press_auto_streets(GtkWidget *, gpointer data){
    
    auto application = static_cast<ezgl::application *>(data);
    
    // get check box for streets
    auto check_strt = (GtkCheckButton *)application->get_object("AutoStreets");
    // if check box is activated don't filter streets in auto-complete
    filter_intersections = !gtk_toggle_button_get_active(&(check_strt->toggle_button));
    
    
    return TRUE;
}


}
