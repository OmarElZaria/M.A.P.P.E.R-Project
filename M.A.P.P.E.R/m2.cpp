/* ECE297 
 *Milestone 2 
 *group team cd 092
 *Authors:
 *Abbas Majed
 *Omar
 * Neehar
 * Scott
 * 
 * Last edited by Abbas : 2/24/2019
*/

#include "m2.h"
#include "m1.h"
#include "global.h"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h" 
#include <cmath>
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "m3.h"

const double PI = 3.14159265;

bool oneWayEnable = false;      //booleans to check if a button is pressed
bool POIenable = false;
bool nearestIntersection = false;
bool nearestPOI = false;
bool distanceFinder = false;
bool firstPoint = true;         //boolean to check if the first point is already stored
bool secondPoint = false;    
unsigned id1;                   //stores id of the first point

bool zoom_level_end = false;    // bool to signal when breaking point of zooming has been passed

float x_from_lon(float lon);    //functions to convert from x and y coords to lat lon and vice versa
float y_from_lat(float lat);
float lon_from_x(float x);
float lat_from_y(float y);

void draw_main_canvas(ezgl::renderer &g);   // main function to directly access the canvas and draw things

void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);

// Activity functions for the run-time buttons
void toggle_POIs_button(GtkWidget *, ezgl::application *application);
void toggle_oneWay_button(GtkWidget *, ezgl::application *application);
void nearest_inter_button(GtkWidget *, ezgl::application *application);
void nearest_POI_button(GtkWidget *, ezgl::application *application);
void help_button(GtkWidget *widget, ezgl::application *application);

// Function to create buttons on run-time
void initial_setup(ezgl::application *application);

void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);

void draw_street_segment(int zoom_level, ezgl::renderer &g, float x_max, float y_max, float x_min, float y_min, bool draw_highlight);
void draw_interest_icon(ezgl::renderer &g, double x_coord, double y_coord);

float x_from_lon(float lon){
    float lat_avg = (((max_lat + min_lat) * DEG_TO_RAD)/2);
    float x = lon * cos(lat_avg) * DEG_TO_RAD;
    return x;
}

float y_from_lat(float lat){
    float y = lat * DEG_TO_RAD;
    return y;
}

float lon_from_x(float x){
    float lat_avg = ((max_lat + min_lat) * DEG_TO_RAD)/2;
    float lon = x/(DEG_TO_RAD * cos(lat_avg));
    return lon;
}

float lat_from_y(float y){
    float lat = y / DEG_TO_RAD;
    return lat;
}

void draw_main_canvas(ezgl::renderer &g){
    
    //allows me to extract numerical values for different zoom levels
    ezgl::rectangle visible = g.get_visible_world();
    double visible_range = (visible.right() - visible.left()) * 1000;
    //std::cout<< visible_range << std::endl;
    std:: string oneWay = "->";
    
    // Determining the position of the viewing window with respect to the map
    double x_min = visible.left();
    double y_min = visible.bottom();
    double x_max = visible.right();
    double y_max = visible.top();
    
    //boolean variables for the different zoom levels
    int zoom_level = 0;
    zoom_level_end = false;
    
    //sets different zoom levels to true depending on the visible range of the screen
    if(visible_range < 11){
        zoom_level = 0;
    }if(visible_range <= 5.98075){
        zoom_level = 1;
    }if(visible_range <= 3.58845){
        zoom_level = 2;
    }if(visible_range <= 2.15307){
        zoom_level = 3;
    }if(visible_range <= 1.29184){
        zoom_level = 4;
    }if(visible_range <= 0.175106){
        zoom_level = 5;
    }if(visible_range <= 0.075){
        zoom_level = 6;
    }if(visible_range <= 0.05){
        zoom_level = 7;
    }if(visible_range <= 0.015){
        zoom_level = 8;
        zoom_level_end = true;
    }
    
    //sets color of canvas to light gray
    g.set_color(220,220,220,255);
    //g.set_color(ezgl::BLACK);
    g.fill_rectangle(visible); 
    
    
    
    // draws all Island and Lake features. Lakes can't be seen above other features.
    for(unsigned i = 0; i < features.size(); i++){
        
        //sets colour and ensures the current feature is in fact an Island; otherwise skips the feature, saving it for the next feature loop
        if(features[i].type() == Island){
            g.set_color(211,211,211,255);
        }
        else if(features[i].type() == Lake){
            g.set_color(ezgl::LIGHT_SKY_BLUE);
        }
        else
            continue;
        
        //declares a vector that holds points in xy coordinates
        std::vector<ezgl::point2d> XY_points;
        //declares the start point and end point of each feature
        LatLon startingPoint = features[i].point(0);
        LatLon endingPoint = features[i].point(features[i].pointCount()-1);
        
        //converts all points of the feature into xy coordinates and stores them in a vector 
        for(unsigned j = 0; j < features[i].pointCount(); j++){
            
            float x = x_from_lon(features[i].point(j).lon());
            float y = y_from_lat(features[i].point(j).lat());
            
            XY_points.push_back({x,y});
        }
        
        //draws the feature based on whether the feature is a point, a closed feature or an open feature
        if(XY_points.size() == 1){
        }
        else if(startingPoint.lon() != endingPoint.lon() && startingPoint.lat() != endingPoint.lat()){
            g.set_line_width(0.5);
            for(unsigned segment = 0; segment < XY_points.size() - 1; segment++){
                g.draw_line (XY_points[segment], XY_points[segment+1]);
            }
        }
        else
            g.fill_poly(XY_points);
    }
    
    
    
    //draws all other features on the map
    //
    for(unsigned i = 0; i < features.size(); i++){
        bool onscreen = false;
        
        //declares a vector that holds points in xy coordinates
        std::vector<ezgl::point2d> XY_points;
        //declares the start point and end point of each feature
        LatLon startingPoint = features[i].point(0);
        LatLon endingPoint = features[i].point(features[i].pointCount()-1);
        
        //converts all points of the feature into xy coordinates and stores them in a vector 
        for(unsigned j = 0; j < features[i].pointCount(); j++){
            
            float x = x_from_lon(features[i].point(j).lon());
            float y = y_from_lat(features[i].point(j).lat());
            
            if((x < x_max && x > x_min) && (y < y_max && y > y_min))
                onscreen = true;
            
            XY_points.push_back({x,y});
        }
        
        //sets a colour for each type of feature and only allows the feature to appear depending on the zoom level
        if(features[i].type() == Park){
            g.set_color(153,255,153,255);
        }else if(features[i].type() == Beach){
            g.set_color(ezgl::KHAKI);
        }else if(features[i].type() == Building && zoom_level >= 4){
            g.set_color(ezgl::GREY_55);
        }else if (features[i].type() == River){
            g.set_color(ezgl::LIGHT_SKY_BLUE);
        }else if (features[i].type() == Greenspace){
            g.set_color(ezgl::GREEN);
        }else if (features[i].type() == Golfcourse){
            g.set_color(153,255,153,255);
        }else if (features[i].type() == Stream && zoom_level >= 3){
            g.set_color(ezgl::LIGHT_SKY_BLUE);
        }else if (features[i].type() == Unknown){
            g.set_color(ezgl::GREY_75);
        }else
            continue;
        
        //draws the feature based on whether the feature is a point, a closed feature or an open feature
        if(onscreen && XY_points.size() == 1){
        }
        else if(onscreen && startingPoint.lon() != endingPoint.lon() && startingPoint.lat() != endingPoint.lat()){
            g.set_line_width(0.5);
            for(unsigned segment = 0; segment < XY_points.size() - 1; segment++){
                g.draw_line (XY_points[segment], XY_points[segment+1]);
            }
        }
        else if(onscreen)
            g.fill_poly(XY_points);
 
    }
    
    
    
    
    //draws all street segments
    //
    // Draw all streets, non-highlighted
    draw_street_segment(zoom_level, g, x_max, y_max, x_min, y_min, false);
    
    // Draw all highlighted street segs
    draw_street_segment(zoom_level, g, x_max, y_max, x_min, y_min, true);
    
    
    
    //draws arrows for one way streets and street names
    //
    for(size_t i = 0; i < street_segments.size(); i++){
        //extracts xy coordinates of each end of the street segment
        float start_x = x_from_lon(street_segments[i].start_position.lon());
        float start_y = y_from_lat(street_segments[i].start_position.lat());
        float end_x = x_from_lon(street_segments[i].end_position.lon());
        float end_y = y_from_lat(street_segments[i].end_position.lat());

        double angle = (180/PI)*(atan((end_y-start_y)/(end_x-start_x)));
        double length = sqrt( (end_y-start_y)*(end_y-start_y) + (end_x-start_x)*(end_x-start_x) );

        bool onscreen = ((start_x < x_max && start_x > x_min) || (end_x < x_max && end_x > x_min))
                        && ((start_y < y_max && start_y > y_min) || (end_y < y_max && end_y > y_min));

        //draws arrows on one way streets when the following conditions are met
        if(onscreen){
            double text_size = 10; // text font size
            
            double passable = 0.1; // passable street length with respect to zoom
            if(zoom_level >= 6)
                passable = 0.1;
            
            // depending on zoom, draw street name an arrow
            if( (zoom_level >= 5) && length > visible.width()*passable){
                g.set_color(ezgl::BLACK);
                g.set_text_rotation(angle);
                g.set_horiz_text_just(ezgl::text_just::center);
                g.set_vert_text_just(ezgl::text_just::center);
                g.format_font("sans serif", ezgl::font_slant::normal, ezgl::font_weight::normal, text_size);
                
                std::string text_out = street_segments[i].street_name;
                
                if(text_out == "<unknown>"){ // do nothing
                }else {
                    if(oneWayEnable && street_segments[i].speedlimit < 80 && street_segments[i].oneWay) // if its a one way street add an arrow
                        text_out = text_out + " " + oneWay;

                    g.draw_text( {(start_x + end_x)/2, (start_y+end_y)/2} , text_out);
                }
            }
        }
    }
    
    
    //draws all highlighted intersections 
    for(size_t i = 0; i < intersections.size(); i++){
        float x = x_from_lon((intersections[i].position.lon()));
        float y = y_from_lat(intersections[i].position.lat());

        float radius = 0.00008;
        
        if(zoom_level >= 0)
            radius = 0.00005;
        if(zoom_level >= 1)
            radius = 0.00003;
        if(zoom_level >= 2)
            radius = 0.000015;
        if(zoom_level >= 3)
            radius = 0.0000075;
        if(zoom_level >= 4)
            radius = 0.000005;
        if(zoom_level >= 5)
            radius = 0.000002;
        if(zoom_level >= 6)
            radius = 0.000001;
            
    
        if ((x < x_max && x > x_min) && (y < y_max && y > y_min) && intersections[i].highlight){
            if(static_cast<int>(i) == start_id){
                g.set_color(ezgl::LIGHT_MEDIUM_BLUE);
                g.fill_arc({x,y}, radius, 0.0, 360.0);
                g.set_color(ezgl::WHITE);
                g.fill_arc({x,y}, radius*0.5, 0.0, 360.0);
            }else{
                g.set_color(ezgl::RED);
                g.fill_arc({x,y}, radius, 0.0, 360.0);
            }
//            draw_interest_icon(g, x, y);
        }
    }
    
    
   //draws out all points of interests
    for(size_t i = 0; i < POIs.size(); i++){
        //finds position of POI
        float x = x_from_lon((POIs[i].position.lon()));
        float y = y_from_lat(POIs[i].position.lat());
    
        float width = 0.0000001;
        float height = width;
        ezgl::point2d centreIcon = {x,y};
        ezgl::surface* currentSurf;
        
        //POIs are set to green when selected 
        if(POIenable && (x < x_max && x > x_min) && (y < y_max && y > y_min)){
            if(POIs[i].highlight){
                if(zoom_level >= 5){
                    g.set_color(ezgl::GREEN);
                    g.fill_rectangle({x,y}, {x + width, y + height});
                }
                if(zoom_level >= 6){
                    // Need to determine which POIs are more relevant --> extra depth of zooming
                    g.set_color(ezgl::BLACK);
                    g.set_text_rotation(0);
                    g.format_font("sans serif", ezgl::font_slant::normal, ezgl::font_weight::normal, 10);
                    g.set_horiz_text_just(ezgl::text_just::right);
                    g.set_vert_text_just(ezgl::text_just::bottom);
                    g.draw_text({x + width/2, y + height*1.1}, POIs[i].name);
                }
            }
            
            else{
                if(zoom_level >= 5){
                    g.set_color(ezgl::RED);
                    g.fill_rectangle({x,y}, {x + width, y + height});
                    
                }
                if(zoom_level >= 7){
                    // Need to determine which POIs are more relevant --> extra depth of zooming
                    // draw arrow
                    g.set_color(ezgl::BLACK);
                    g.set_text_rotation(0);
                    g.format_font("sans serif", ezgl::font_slant::normal, ezgl::font_weight::normal, 10);
                    g.set_horiz_text_just(ezgl::text_just::right);
                    g.set_vert_text_just(ezgl::text_just::bottom);
                    g.draw_text({x + width/2, y + height*1.1}, POIs[i].name);
                    if(POIs[i].type == "fast_food"){
                        currentSurf = g.load_png("/homes/k/kantimah/ece297/work/mapper/libstreetmap/resources/fast_food");
                        g.draw_surface(currentSurf, centreIcon);
                        g.free_surface(currentSurf);
                    }
                    if(POIs[i].type == "school" || POIs[i].type == "college"){
                        currentSurf = g.load_png("/homes/k/kantimah/ece297/work/mapper/libstreetmap/resources/school");
                        g.draw_surface(currentSurf, centreIcon);
                        g.free_surface(currentSurf);
                    }
                    if(POIs[i].type == "fuel"){
                        currentSurf = g.load_png("/homes/k/kantimah/ece297/work/mapper/libstreetmap/resources/fuel");
                        g.draw_surface(currentSurf, centreIcon);
                        g.free_surface(currentSurf);
                    }
                    if(POIs[i].type == "cinema"){
                        currentSurf = g.load_png("/homes/k/kantimah/ece297/work/mapper/libstreetmap/resources/cinema");
                        g.draw_surface(currentSurf, centreIcon);
                        g.free_surface(currentSurf);
                    }
                    if(POIs[i].type == "bus_station"){
                        currentSurf = g.load_png("/homes/k/kantimah/ece297/work/mapper/libstreetmap/resources/bus_station");
                        g.draw_surface(currentSurf, centreIcon);
                        g.free_surface(currentSurf);
                    }
                    if(POIs[i].type == "bank"){
                        currentSurf = g.load_png("/homes/k/kantimah/ece297/work/mapper/libstreetmap/resources/bank");
                        g.draw_surface(currentSurf, centreIcon);
                        g.free_surface(currentSurf);
                    }
                    if(POIs[i].type == "restaurant"){
                        currentSurf = g.load_png("/homes/k/kantimah/ece297/work/mapper/libstreetmap/resources/restaurant");
                        g.draw_surface(currentSurf, centreIcon);
                        g.free_surface(currentSurf);
                    }
                    if(POIs[i].type == "cafe"){
                        currentSurf = g.load_png("/homes/k/kantimah/ece297/work/mapper/libstreetmap/resources/cafe");
                        g.draw_surface(currentSurf, centreIcon);
                        g.free_surface(currentSurf);
                    }
                    if(POIs[i].type == "atm"){
                        currentSurf = g.load_png("/homes/k/kantimah/ece297/work/mapper/libstreetmap/resources/atm");
                        g.draw_surface(currentSurf, centreIcon);
                        g.free_surface(currentSurf);
                    }
                    if(POIs[i].type == "hospital"){
                        currentSurf = g.load_png("/homes/k/kantimah/ece297/work/mapper/libstreetmap/resources/");
                        g.draw_surface(currentSurf, centreIcon);
                        g.free_surface(currentSurf);
                    }
                }
            }
        }
    }
    
}

void toggle_POIs_button(GtkWidget *, ezgl::application *application){
    POIenable = !POIenable;
    
    application -> refresh_drawing();
}

void toggle_oneWay_button(GtkWidget *, ezgl::application *application){
    oneWayEnable = !oneWayEnable;
    
    application -> refresh_drawing();
}

void nearest_inter_button(GtkWidget *, ezgl::application *application){
    nearestIntersection = true;
    
    application -> refresh_drawing();
}

void nearest_POI_button(GtkWidget *, ezgl::application *application){
    nearestPOI = true;
    
    application -> refresh_drawing();
}

void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer ){
// For demonstration purposes, this will show the enum name and int value of the button that was pressed
    std::cout << "response is ";
    switch(response_id){
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
    // This will cause the dialog to be destroyed and close
    // without this line the dialog remains open unless the
    // response_id is GTK_RESPONSE_DELETE_EVENT which
    // automatically closes the dialog without the following line.
    gtk_widget_destroy(GTK_WIDGET (dialog));
}

void initial_setup(ezgl::application *application){
    application -> create_button("Toggle POI", 10, toggle_POIs_button);
    application -> create_button("Toggle One-Ways", 11, toggle_oneWay_button);
    application -> create_button("Nearest Inter", 12, nearest_inter_button);
    application -> create_button("Nearest POI", 13, nearest_POI_button);
}

void act_on_mouse_click(ezgl::application* app, GdkEventButton* , double x, double y){
    //displays closest intersection from the user's current location
    if(nearestIntersection){
        start_id = NO_VALUE;
        // Turn off previous highlights (this may seem really slow)
        for(unsigned i = 0; i < intersections.size(); i++)
            intersections[i].highlight = false;
        for(unsigned i = 0; i < street_segments.size(); i++)
            street_segments[i].highlight = false;  
        
        std::string Inter;

        std::cout << "Mouse clicked at (" << x << "," << y << ")\n";

//        ezgl::point2d centreIcon = {x,y};
        LatLon pos = LatLon(lat_from_y(y), lon_from_x(x)); 
        int id = find_closest_intersection(pos); 

        intersections[id].highlight = true;       
        
        Inter = "Closest Intersection: " + intersections[id].name;
        
        app->update_message(Inter);
        app->refresh_drawing();
        nearestIntersection = false;
    }
    
    //displays the closest POI from the user's current location
    if(nearestPOI){
        start_id = NO_VALUE;
        for(unsigned i = 0; i < POIs.size(); i++)
            POIs[i].highlight = false;  
        std::string POI;
        
        std::cout << "Mouse clicked at (" << x << "," << y << ")\n";

        LatLon pos = LatLon(lat_from_y(y), lon_from_x(x)); 
        int id = find_closest_point_of_interest(pos); 

        POIs[id].highlight = true;

        POI = "Closest Point of Interest: " + POIs[id].name;
        
        app->update_message(POI);
        app->refresh_drawing();
        nearestPOI = false;
    }
    
    //displays distance between two points 
    if(distanceFinder){
        // Get the GtkEntry object
        GtkEntry* text_entry1 = (GtkEntry *) app->get_object("FindStreet1");
        GtkEntry* text_entry2 = (GtkEntry *) app->get_object("FindStreet2");

        // BEGIN: CODE FOR SHOWING 
        GtkWidget *new_window = NULL;         // the parent window over which to add the dialog
        GtkWidget *scrolled_window = NULL;
        GtkWidget *label = NULL; // the label we will create to display a message in the content area

        if(firstPoint){
        std::cout << "Mouse first clicked at (" << x << "," << y << ") \n";
        
        LatLon position1 = LatLon(lat_from_y(y), lon_from_x(x));
        id1 = find_closest_intersection(position1);
        start_id = id1;

        intersections[id1].highlight = true;
        gtk_entry_set_text(GTK_ENTRY(text_entry1), &intersections[id1].name[0]); 
        
        app->refresh_drawing();
        firstPoint = false;
        secondPoint = true;
        }
        else if(secondPoint){  //if first click has already occurred
        std::cout << "Mouse then clicked at (" << x << "," << y << ") \n";
            
        LatLon position2 = LatLon(lat_from_y(y), lon_from_x(x));
        unsigned id2 = find_closest_intersection(position2); 
            
        intersections[id2].highlight = true;
        
        
        std::vector<unsigned> path_directions = find_path_between_intersections(id1, id2, 15, 25);
        
        gtk_entry_set_text(GTK_ENTRY(text_entry2), &intersections[id2].name[0]); 
        
        int travel_time = (compute_path_travel_time(path_directions, 15, 25))/60;

        
        //looping through all the street segments in the path and highlighting the ids in street_segments that correspond to those in the path directions vector
        for(unsigned i=0; i < path_directions.size(); i++){
            street_segments[path_directions[i]].highlight = true;

        }

		                
        
        std::string travel_directions;
        double current_street_segment_length;
        unsigned current_id;
        double total_length=0;  // used to accumulate the street length in case of straight turns 
        
        //looping through the path directions and adding to the string that stores all the directions to be printed
        for(unsigned k = 0; k < path_directions.size() - 1; k++){
            std::string turn;
            bool next_turn_exists = false;
            TurnType next_turn = TurnType::NONE;
            TurnType temp = find_turn_type(path_directions[k], path_directions[k + 1]);
            
            if(k < path_directions.size() - 2){
                next_turn = find_turn_type(path_directions[k + 1], path_directions[k + 2]);
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
            InfoStreetSegment current = getInfoStreetSegment(path_directions[k]);
            InfoStreetSegment next = getInfoStreetSegment(path_directions[k + 1]);
            current_id = current.streetID;
            unsigned next_id = next.streetID;
            std::string current_street = getStreetName(current_id);
            std::string next_street = getStreetName(next_id);
            current_street_segment_length = find_street_segment_length(path_directions[k]);
//            double next_street_segment_length = find_street_segment_length(path_directions[k + 1]);
            
            // we have 3 cases Straight then Straight, Straight then Turn, or Turn 
            
            if(turn == "straight"){ 
                if(next_turn == TurnType::STRAIGHT && next_turn_exists){    
                                       
                    // in case of double straight , we need to accumulate the lengths without printing 
                    total_length +=current_street_segment_length;
                    
                }else{
                    if (total_length == 0){
                    travel_directions += "Continue straight on "; 
                    travel_directions += current_street; 
                    travel_directions += " for "; 
                    travel_directions += std::to_string(static_cast<int>(current_street_segment_length)); 
                    travel_directions += " m \n\n";
                    }     
                   

                    else {
                        int travel_length = (int)total_length+(int)current_street_segment_length;
                        travel_directions += "Continue straight on "; 
                        travel_directions += current_street; 
                        travel_directions += " for "; 
                        travel_directions += std::to_string(travel_length);  // add last length to previous accumulated ones before next turn
                        travel_directions += " m \n";
                        
                        total_length=0; // reset value to Zero since a turn right or left is coming next
                
                    }
                }    
                        
            }
            
            // if turn is left or right
            else{
               travel_directions += "Turn "; 
               travel_directions += turn ;
               travel_directions +=  " onto "; 
               travel_directions += next_street;
               travel_directions += " \n";

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
            
        
        
        app->refresh_drawing();
        secondPoint = false;
        firstPoint = true;
        distanceFinder = false;
        } 
        
    }
}

// Main graphics function
void draw_map(){
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";

    ezgl::application application(settings);

    ezgl::rectangle initial_world({x_from_lon(min_lon), y_from_lat(min_lat)},{x_from_lon(max_lon), y_from_lat(max_lat)});
    
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);

    application.run(initial_setup, act_on_mouse_click, nullptr, nullptr);
}

/** Take the renderer and draw street segments that are in frame. draw_highlight toggles drawing highlighted or non highlighted segments.
 */
void draw_street_segment(int zoom_level, ezgl::renderer &g, float x_max, float y_max, float x_min, float y_min, bool draw_highlight){
    for(size_t i = 0; i < street_segments.size(); i++){
        if(!draw_highlight && street_segments[i].speedlimit >= 80) // If high speed limit, colour is bright orange
            g.set_color(255,215,0,255);
        else if(!draw_highlight)
            g.set_color(ezgl::WHITE);
        else
            g.set_color(ezgl::LIGHT_MEDIUM_BLUE);
        
        //extracts xy coordinates of each end of the street segment
        float start_x = x_from_lon(street_segments[i].start_position.lon());
        float start_y = y_from_lat(street_segments[i].start_position.lat());
        float end_x = x_from_lon(street_segments[i].end_position.lon());
        float end_y = y_from_lat(street_segments[i].end_position.lat());
        
        // Determine if the start and end points of the road are visible
        bool onscreen = ((start_x < x_max && start_x > x_min) || (end_x < x_max && end_x > x_min))
                            && ((start_y < y_max && start_y > y_min) || (end_y < y_max && end_y > y_min));
        
        // Draw street segments with varying thickness depending on speed limits: //
        
        if((draw_highlight && street_segments[i].highlight) || !draw_highlight){
            //draws all street segments with no curve points.
            if(onscreen && street_segments[i].curvePointCount == 0){

                //draws different street segment thickness and colour depending on the speed limit
                if(street_segments[i].speedlimit >= 80){
                    g.set_line_width(1);
                    if(zoom_level >= 0){
                        g.set_line_width(2);    
                    }
                    if(zoom_level >= 2){
                        g.set_line_width(4);
                    }
                    if(zoom_level >= 5){
                        g.set_line_width(12);
                    }
                    if(zoom_level >= 6)
                        g.set_line_width(24);
                    
                    if(draw_highlight)
                        g.set_line_width(3);

                    g.set_line_cap (ezgl::line_cap::round);
                    g.draw_line({start_x, start_y}, {end_x, end_y}); 
                }
                else if(street_segments[i].speedlimit >= 60 && street_segments[i].speedlimit < 80){
                    g.set_line_width(1);
                    if(zoom_level >= 0){
                        g.set_line_width(2);
                    }
                    if(zoom_level >= 2){
                        g.set_line_width(4);
                    }
                    if(zoom_level >= 5){
                        g.set_line_width(10);
                    }
                    if(zoom_level >= 6)
                        g.set_line_width(20);
                    
                    if(draw_highlight)
                        g.set_line_width(3);

                    g.set_line_cap (ezgl::line_cap::round);
                    g.draw_line({start_x, start_y}, {end_x, end_y});   
                }
                else{
                    g.set_line_width(1);
                    if(zoom_level >= 3)
                        g.set_line_width(2);
                    if(zoom_level >= 5)
                        g.set_line_width(6);
                    if(zoom_level >= 6)
                        g.set_line_width(16);
                    
                    if(draw_highlight)
                        g.set_line_width(3);

                    if(zoom_level >= 3 || draw_highlight){
                        g.set_line_cap (ezgl::line_cap::round);
                        g.draw_line({start_x, start_y}, {end_x, end_y});
                    }
                }

            }else {

                //draws all street segments with curve points 
                std::vector<ezgl::point2d> XY_curvePoints;

                // Build curvepoints vector
                for(int j = 0; j < street_segments[i].curvePointCount; j++){
                    float x = x_from_lon(getStreetSegmentCurvePoint(j, i).lon());
                    float y = y_from_lat(getStreetSegmentCurvePoint(j, i).lat());

                    XY_curvePoints.push_back({x,y});

                    if((x < x_max && x > x_min) && (y < y_max && y > y_min))
                        onscreen = true; // if any curve points are visible, set to true
                }

                // Draw the curved segment
                //
                if(onscreen){
                    //draws different street segment thickness and color depending on the speed limit
                    if(street_segments[i].speedlimit >= 80){
                        // set line width
                        //
                        g.set_line_width(2);
                        if(zoom_level == 0){
                            g.set_line_width(2);    
                        }
                        if(zoom_level >= 1){
                            g.set_line_width(4);  
                        }
                        if(zoom_level >= 3){
                            g.set_line_width(6);  
                        }
                        if(zoom_level >= 5){
                            g.set_line_width(12);  
                        }
                        if(zoom_level >= 6)
                            g.set_line_width(24);
                        
                        if(draw_highlight)
                            g.set_line_width(3);

                        g.set_line_cap (ezgl::line_cap::round);
                        g.draw_line({start_x, start_y}, XY_curvePoints[0]);
                        for(size_t j = 0; j < XY_curvePoints.size() - 1; j++){
                            g.draw_line (XY_curvePoints[j], XY_curvePoints[j + 1]);
                        }
                        g.draw_line(XY_curvePoints[XY_curvePoints.size()-1],{end_x, end_y});
                    }
                    else if(street_segments[i].speedlimit >= 60 && street_segments[i].speedlimit < 80){
                        //set line width
                        //
                        g.set_line_width(2);
                        if(zoom_level >= 0){
                            g.set_line_width(2);  
                        }
                        if(zoom_level >= 2){
                            g.set_line_width(4);
                        }
                        if(zoom_level >= 5){
                            g.set_line_width(10);
                        }
                        if(zoom_level >= 6)
                            g.set_line_width(20);
                        
                        if(draw_highlight)
                            g.set_line_width(3);

                        // draw every part of the curve
                        //
                        g.set_line_cap (ezgl::line_cap::round);
                        g.draw_line({start_x, start_y}, XY_curvePoints[0]);
                        for(size_t j = 0; j < XY_curvePoints.size() - 1; j++){
                            g.draw_line (XY_curvePoints[j], XY_curvePoints[j + 1]);
                        }
                        g.draw_line(XY_curvePoints[XY_curvePoints.size()-1],{end_x, end_y});
                    }else{
                        // set line width
                        g.set_line_width(2);
                        if(zoom_level >= 3){
                            g.set_line_width(2);
                        }
                        if(zoom_level >= 5){
                            g.set_line_width(6);
                        }
                        if(zoom_level >= 6){
                            g.set_line_width(16);
                        }
                        
                        if(draw_highlight)
                            g.set_line_width(3);

                        // draw every part of the curve if necessary
                        if(zoom_level >= 3 || draw_highlight){
                            g.set_line_cap (ezgl::line_cap::round);
                            g.draw_line({start_x, start_y}, XY_curvePoints[0]);
                            for(size_t j = 0; j < XY_curvePoints.size() - 1; j++){
                                g.draw_line (XY_curvePoints[j], XY_curvePoints[j + 1]);
                            }
                            g.draw_line(XY_curvePoints[XY_curvePoints.size()-1],{end_x, end_y});
                        }
                    }
                }
            }
        }
    }
}

// Draw the interest icon based on the given x and y coordinates
void draw_interest_icon(ezgl::renderer &g, double x_coord, double y_coord){
    double icon_width = 13.0;
    ezgl::point2d centreIcon(x_coord - icon_width/2, y_coord);
    ezgl::surface* currentSurf = g.load_png("libstreetmap/resources/small_image.png");
    g.draw_surface(currentSurf, centreIcon);
    g.free_surface(currentSurf);
}

