/* 
 * ECE297 Winter 2019
 * group cd-092
 * 
 * Authors:
 * Abbas Majed
 * Omar
 * Neehar
 * Scott
 * 
 * February 3 2019
 * Time last edited : 7:00 pm
 */
#include "m1.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include <sstream>
#include "global.h"

double max_lat = -1000;   //initializing min and max values
double min_lat = 1000;
double max_lon = -1000;
double min_lon = 1000;

int start_id = NO_VALUE;

bool filter_streets = false;
bool filter_intersections = false;

std::vector<intersection_data> intersections; // hold every intersection's data
std::vector<POI_data> POIs; // hold all POI data
std::vector<Feature> features; // hold every feature
std::vector<street_segment_data> street_segments; // hold all street segment data

std::string toLowerCase(std::string str);
double find_street_segment_travel_time_helper(unsigned street_segment_id);

struct map_data_structures* map_data = NULL;
/** Converts all chars in str to lower case letters if they are upper case letters.
 */
std::string toLowerCase(std::string str){
    // Go through every character in str, use tolower() on them
    for(unsigned int i=0; i < str.length(); i++)
        str[i] = tolower(str[i]);
    return str;
}


// Function #1 : LOAD MAP

//Loads a map streets.bin file. Returns true if successful, false if some error
//occurs and the map can't be loaded.
bool load_map(std::string map_name) {
    
    //Load map data. Exit load_map if the load isn't successful
    if(!loadStreetsDatabaseBIN(map_name))
        return false;
    // If loading was  a success, then set up the map data structure
    map_data = new struct map_data_structures;
    
    unsigned int size = getNumStreets(); // using variable to hold size reduces function calls
    
    // Load data structures which require streets
    std::cout << "Loading Streets..." << std::endl;
    for(unsigned int i=0; i < size; i++){
        
        // Load strt_segs_ID_list with empty vectors corresponding to each street
        std::vector<unsigned> temp;
        
        map_data->strt_segs_ID_list.push_back(temp); // not sure how well this works
        
        // Load list_strt_intersections with vectors for each street
        map_data->list_strt_intersections.push_back(temp);
        
        std::unordered_set<unsigned> hash_temp;
        // load hash_strt_intersections with hash tables for each street
        map_data->hash_strt_intersections.push_back(hash_temp);
        
        // Load strt_name_and_ID with every street's name (in lower case) and ID
        map_data->strt_name_and_ID.insert({toLowerCase(getStreetName(i)), i});
    }
    
    // Load data structures requiring intersections, incorporating segments
    size = getNumIntersections();
    
    std::cout << "Loading Intersection Data Structures..." << std::endl;
    for(unsigned int i=0; i < size; i++){
        std::vector<unsigned> temp;
	map_data->inter_strt_segs_list.push_back(temp); // builds the vector of vectors of segment IDs indexed by inter IDs
        
        // Load inter_name_nad_ID with every intersection's name (in lower case) and ID
        map_data->inter_name_and_ID.insert({toLowerCase(getIntersectionName(i)), i});
        
    }
    
    size = getNumStreetSegments();
    std::cout << "Loading Street Segments and Intersections..." << std::endl;
    // load segments and finish inter graph
    for(unsigned int i=0; i < size; i++){
	InfoStreetSegment temp = getInfoStreetSegment(i); // get all the info about current segment

	// push segment id into the vectors at inter from and to index. Done building inter_strt_segs_list
	map_data->inter_strt_segs_list[temp.from].push_back(i);
	map_data->inter_strt_segs_list[temp.to].push_back(i);

	// push segment id into the vector at street ID index
	map_data->strt_segs_ID_list[temp.streetID].push_back(i);
        
        // push travel time into the corresponding segment ID index i
        map_data->strt_seg_travel_time.push_back(find_street_segment_travel_time_helper(i));
        
        // Build list_strt_intersections by looking at the two intersections connected to the current segment
        //
        // Check if the 'from' intersection has been added to the strt intersections data structures
        std::unordered_set<unsigned>::iterator it = map_data->hash_strt_intersections[temp.streetID].find(temp.from);
        if(it == map_data->hash_strt_intersections[temp.streetID].end()){
            // update strt intersections list and hash table accordingly
            map_data->list_strt_intersections[temp.streetID].push_back(temp.from);
            map_data->hash_strt_intersections[temp.streetID].insert(temp.from);
        }
        
        // Check if the 'to' intersection has been added to the strt intersections data structure
        it = map_data->hash_strt_intersections[temp.streetID].find(temp.to);
        if(it == map_data->hash_strt_intersections[temp.streetID].end()){
            // update strt intersections list and hash table accordingly
            map_data->list_strt_intersections[temp.streetID].push_back(temp.to);
            map_data->hash_strt_intersections[temp.streetID].insert(temp.to);
        }
        
        // Build TurnType data structure
        //
        std::unordered_map<unsigned, TurnType> neighbourSegTurns;
        // Traveling to 'to' intersection
        int intersectionSize = getIntersectionStreetSegmentCount(temp.to);
        for(int j=0; j < intersectionSize; j++){
            unsigned neighbour = getIntersectionStreetSegment(j, temp.to);
            neighbourSegTurns.insert({neighbour, find_turn_type(i, neighbour)});
        }
        // Continue building TurnType data structure
        // Traveling to 'from' intersection if not oneway
        if(!temp.oneWay){
            intersectionSize = getIntersectionStreetSegmentCount(temp.from);
            for(int j=0; j < intersectionSize; j++){
                unsigned neighbour = getIntersectionStreetSegment(j, temp.from);
                neighbourSegTurns.insert({neighbour, find_turn_type(i, neighbour)});
            }
        }
      
        // push back hashmap nto seg turns data structure
        map_data->segment_turns.push_back(neighbourSegTurns);
    }
    
    std::cout << "Creating auxillary data structures..." << std::endl;
    
    // Reset latitude and longitude extremes
    max_lat = -1000; 
    min_lat = 1000;
    max_lon = -1000;
    min_lon = 1000;
    start_id = NO_VALUE;
    
    intersections.resize(getNumIntersections());
    POIs.resize(getNumPointsOfInterest());
    features.resize(getNumFeatures());
    street_segments.resize(getNumStreetSegments());
    
    for(int i = 0; i < getNumIntersections(); i++){
        intersections[i].id = i;
        intersections[i].position = getIntersectionPosition(i);   
        intersections[i].name = getIntersectionName(i);
        intersections[i].highlight = false;
        
        if(max_lat < intersections[i].position.lat()){
            max_lat = intersections[i].position.lat();
        }if(min_lat > intersections[i].position.lat()){
            min_lat = intersections[i].position.lat();
        }if(max_lon < intersections[i].position.lon()){
            max_lon = intersections[i].position.lon();
        }if(min_lon > intersections[i].position.lon()){
            min_lon = intersections[i].position.lon();
        }
    }
    
    for(int i = 0; i < getNumStreetSegments(); i++){
        InfoStreetSegment temp = getInfoStreetSegment(i);
        street_segments[i].id = static_cast<unsigned>(i);
        street_segments[i].start_position = getIntersectionPosition(temp.from);
        street_segments[i].end_position = getIntersectionPosition(temp.to);
        street_segments[i].length = find_street_segment_length(i);
        street_segments[i].curvePointCount = temp.curvePointCount;
        street_segments[i].speedlimit = temp.speedLimit;
        street_segments[i].street_name = getStreetName(getInfoStreetSegment(i).streetID);
        street_segments[i].oneWay = temp.oneWay;
        street_segments[i].highlight = false;
    }
    
    for(int i = 0; i < getNumPointsOfInterest(); i++){
        POIs[i].position = getPointOfInterestPosition(i);
        POIs[i].name = getPointOfInterestName(i);
        POIs[i].type = getPointOfInterestType(i);
        POIs[i].highlight = false;
    }
    
    for(int id = 0; id < getNumFeatures(); id++){
        unsigned pt_count =  getFeaturePointCount(id);
        
        std::string CurrentFeatureName = getFeatureName(id);
        std::vector<LatLon> points;
        
        points.resize(pt_count);
        for(unsigned i = 0; i < pt_count; i++){
            points[i] = getFeaturePoint(i, id);
        }
        
        Feature CurrentFeature = Feature(getFeatureOSMID(id), getFeatureType(id), move(CurrentFeatureName), move(points), true);
        
        features[id] = CurrentFeature;
        
        points.clear();
    }

    std::cout << "Done load_map." << std::endl;
    
    //re initialize filter_streets
    filter_streets = false;
    filter_intersections = false;
    
    //load_map was a success!!
    return true;
}

// Function #2 : CLOSE MAP

//Close the map (if loaded)
void close_map(){    
    // Delete map data structure and close the bin
    delete map_data;
    map_data = NULL;
    closeStreetDatabase();
}

// Function #3 : Intersection Street Names 

//Returns the street names at the given intersection (includes duplicate street 
//names in returned vector)
std::vector<std::string> find_intersection_street_names(unsigned intersection_id){
   std::vector<std::string> names;
   
   //looping through the intersection street segments

   for(unsigned i = 0; i < map_data->inter_strt_segs_list[intersection_id].size(); i++){                    //looping through the intersection street segments
       InfoStreetSegment temp = getInfoStreetSegment(map_data->inter_strt_segs_list[intersection_id][i]);   //getting the street segment info
       
       names.push_back(getStreetName(temp.streetID));                                  //storing the street names in the intersection
   }
   return names;
}

// Function #4 : Intersection Street segments 

//Returns the street segments for the given intersection 
std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id){
    return map_data->inter_strt_segs_list[intersection_id];
}

//Returns the travel time to drive a street segment in seconds 
//(time = distance/speed_limit)
// Actually does the calculations to determine travel time.
double find_street_segment_travel_time_helper(unsigned street_segment_id){
   double length = find_street_segment_length(street_segment_id)/1000;        //getting the length of the street segment in km             
   
   InfoStreetSegment temp = getInfoStreetSegment(street_segment_id);             //getting the street segment info
   
   double time = (length/(temp.speedLimit))*3600;                                //finding the travel time in minutes
   
   return time;
}

// Function #5 : Street Segment Travel Time 

//Returns the travel time to drive a street segment in seconds, as recorded in the data structures
double find_street_segment_travel_time(unsigned street_segment_id){
    return map_data->strt_seg_travel_time[street_segment_id];
}

// Function #6 : Street Length

//Returns the length of the specified street in meters

double find_street_length(unsigned street_id){
    double length = 0;
   
    for(unsigned i = 0; i < map_data->strt_segs_ID_list[street_id].size(); i++){                      //looping through the street segments
        length += find_street_segment_length(map_data->strt_segs_ID_list[street_id][i]);   //adding up all the street segment lengths
    }
    
    return length;                                                               //returning the total length of the street
}

// Function #7 : Intersections Connection 

//Returns true if you can get from intersection1 to intersection2 using a single 
//street segment (hint: check for 1-way streets too)
//corner case: an intersection is considered to be connected to itself

bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2){
    unsigned size  = map_data->inter_strt_segs_list[intersection_id1].size();
    
    //if the intersection id is the same, then it's just a point
    if(intersection_id1 == intersection_id2){   
        return true;
    }
    
    for(unsigned int i = 0; i < size; i++){
        InfoStreetSegment temp = getInfoStreetSegment(map_data->inter_strt_segs_list[intersection_id1][i]);
        
        
        if(temp.oneWay == false){    //checks if the streets are connected when it's not a one way street
            if((static_cast<unsigned>(temp.from) == intersection_id1 || static_cast<unsigned>(temp.from) == intersection_id2) && (static_cast<unsigned>(temp.to) == intersection_id2 || static_cast<unsigned>(temp.from) == intersection_id2)) 
                return true;
        }else if(temp.oneWay == true){   //checks if the streets are connected when it's a one way street
            if(static_cast<unsigned>(temp.to) == intersection_id2)
                return true;
        }
    }
    
    return false;
}

// Function #8 :  Street StreetSegments

//Returns all street segments for the given street
std::vector<unsigned> find_street_street_segments(unsigned street_id){
    return map_data->strt_segs_ID_list[street_id];
}


// Function #9 : Distance of TWO Points

//Returns the distance between two coordinates in meters
double find_distance_between_two_points(LatLon point1, LatLon point2){
    double distance, lat_avg, x ,y;
    
    //average latitude between point 1 and point 2 
    lat_avg = ((point1.lat()+point2.lat())* DEG_TO_RAD)/2;
    
    //x and y coordinates 
    x = static_cast<double> (point2.lon() * DEG_TO_RAD * std::cos(lat_avg)) - static_cast<double> (point1.lon() * DEG_TO_RAD * std::cos(lat_avg));
    y = static_cast<double> (point2.lat() * DEG_TO_RAD) - static_cast<double> (point1.lat() * DEG_TO_RAD);
    
    //formula to calculate the distance between 2 points on the map
    distance = EARTH_RADIUS_IN_METERS * std::sqrt(y*y + x*x);
    
    return distance;
}

// Function #10 : Street Intersections

//Returns all intersections along the a given street
std::vector<unsigned> find_all_street_intersections(unsigned street_id){
    return map_data->list_strt_intersections[street_id];
}

// Function #11 : Intersections of TW0 Streets


//Return all intersection ids for two intersecting streets
//This function will typically return one intersection id.
std::vector<unsigned> find_intersection_ids_from_street_ids(unsigned street_id1, unsigned street_id2){
    // create two vectors for both streets from the given street ids
    
    std::vector<unsigned> street1 = find_all_street_intersections(street_id1);   // street1intersections contains all street1 intersections ids
    std::vector<unsigned> street2 = find_all_street_intersections(street_id2);   // street2 contains all street2 intersections ids
    
    // vectors will be compared to find a common intersection id
    
    std::vector<unsigned> commonids;  // commonids is an empty vector to be filled with a common id intersection between street1 and street2
    
    for (unsigned i=0; i< street1.size(); i++){          // i represent index iteration for street1 vector of intersection ids
        for (unsigned j=0; j<street2.size(); j++){       // j represent index iteration for street2 vector of intersection ids
            if (street1[i]==street2[j]) {
                commonids.push_back(street1[i]);            // insert common intersection id found between both streets to keep track of intersection ids
            }
        }
    }
    
    return commonids;
}

// Function #12 : Adjacent Intersections


//Returns all intersections reachable by traveling down one street segment 
//from given intersection (hint: you can't travel the wrong way on a 1-way street)
//the returned vector should NOT contain duplicate intersections
std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id){
    // create empty vector to be filled with adjacent intersections to the given intersection
    std::vector<unsigned> adjacentintersections;    
    
    for(unsigned i = 0; i < map_data->inter_strt_segs_list[intersection_id].size(); i++){                    //looping through the intersection street segments
       InfoStreetSegment temp = getInfoStreetSegment(map_data->inter_strt_segs_list[intersection_id][i]); 
       
       unsigned intersection1 = temp.from;    // used to convert segment intersection index into unsigned id
       unsigned intersection2 = temp.to;
       
       // check for one-way segment
          
       if ((temp.oneWay) && ( intersection1 == intersection_id)) {   // true can only travel from->to direction
           adjacentintersections.push_back(temp.to);          // add the adjacent intersection to the vector
        }
       else if (temp.oneWay == false) {                        // false two-way direction so should be careful to which intersection is to or from
           if (intersection2 == intersection_id){
               adjacentintersections.push_back(intersection1);
           }
           else if (intersection1 == intersection_id){
               adjacentintersections.push_back(intersection2);
           }
       }
    }
    
    std::sort(adjacentintersections.begin(), adjacentintersections.end()); // 1 1 2 2 3 3 3 4 4 5 5 6 7 
    auto last = std::unique(adjacentintersections.begin(), adjacentintersections.end());
    // v now holds {1 2 3 4 5 6 7 x x x x x x}, where 'x' is indeterminate
    adjacentintersections.erase(last, adjacentintersections.end());
    
    return adjacentintersections; 
}


// Function #13 : StreetSegment Length


//Returns the length of the given street segment in meters
double find_street_segment_length(unsigned street_segment_id){
    double length = 0;
    LatLon temp;
    InfoStreetSegment segment = getInfoStreetSegment(street_segment_id);           //storing the segment info
    LatLon start = getIntersectionPosition(segment.from);                      //getting the start and end position of the segment
    LatLon end = getIntersectionPosition(segment.to);
    
    if(segment.to == segment.from && segment.curvePointCount == 0){
        length = 0;
    }
    else if(segment.curvePointCount == 0){                                              //if there are no curve points
        length = find_distance_between_two_points(start, end);               //finding the distance between the start and end positions
    }
    else{                                                                       //if there are more than one curve points
        for(int i = 0; i < segment.curvePointCount; i++){
            LatLon curvePoint = getStreetSegmentCurvePoint(i, street_segment_id);
            if(i == 0){
                length += find_distance_between_two_points(start, curvePoint);  
            }
            else{
                length += find_distance_between_two_points(temp, curvePoint);  //finding the distances between the curve points
            }
            temp = curvePoint;
        }  
        length += find_distance_between_two_points(temp, end);            //finding the distance from the last curve point to the end of the segment
    }
    return length;
}

// Function #14 : PointofInterest AND Position 

//Returns the nearest point of interest to the given position
unsigned find_closest_point_of_interest(LatLon my_position){
    // Declarations; distances initialized to negative values
    unsigned poi_index = 0;                                                     // poi is point of interest
    double distance = -1;                                                       
    double temp_dist = -1;                                                      
    const int numPOI = getNumPointsOfInterest();                                
    
    // go through all points of interest and find distance from each one
    // compiler isnt happy with unsigned and int comparison
    for(int i = 0; i < numPOI; i++){                                       
        // get distance from current poi
        temp_dist = find_distance_between_two_points(my_position, getPointOfInterestPosition(i));
        
        // check if a new minimum distance
        if(temp_dist < distance || distance < 0){
            poi_index = i;                                                      
            distance = temp_dist;                                               
        }
    }
    // At this point we will have the index of the closest POI
    return poi_index;
}

// Function #15 : Intersections AND Position 


//Returns the nearest intersection to the given position
unsigned find_closest_intersection(LatLon my_position){
    // Declarations; distances initialized to negative values
    unsigned inter_index = 0;                                                   
    double distance = -1;                                                       
    double temp_dist = -1;                                                      
    const int numInter = getNumIntersections();                                 // hold number of intersections. reduces function calls
    
    // iterate through all intersections and get the distance from each
    for(int i = 0; i < numInter; i++){                                        
        // Get distance from current intersection
        temp_dist = find_distance_between_two_points(my_position, getIntersectionPosition(i));
        
        // check if there is a new minimum distance
        if(temp_dist < distance || distance < 0){
            inter_index = i;                                                       
            distance = temp_dist;                                               
        }
    }
    // At this point we will have the index of the closest intersection
    return inter_index;
}

// Function #16 : Streets from Partial Names

//Returns all street ids corresponding to street names that start with the given prefix
//For example, both "bloo" and "BloO" are prefixes to "Bloor Street East".
//If no street names match the given prefix, this routine returns an empty (length 0) 
//vector.
std::vector<unsigned> find_street_ids_from_partial_street_name(std::string street_prefix){
    std::vector<unsigned> streetIDs; // The container to return
    street_prefix = toLowerCase(street_prefix); // convert street_prefix to lower case so it works with the multimap and char manipulation.
    
    // start iterator for the lower bound of the keys in street_prefix range. Used to iterator over all street names.
    std::multimap<std::string, unsigned>::iterator start_it = map_data->strt_name_and_ID.lower_bound(street_prefix);
    
    // Incrementing the last char in the string by 1 and lower_bound the iterator with the new string, which upper bounds the prefix names.
    street_prefix[street_prefix.length() - 1]++;
    // Assign end_it to the lower bound of the large value string/key, which results in a more reliable upper_bound. I.e. prefix <= key < prefix (last char + 1)
    std::multimap<std::string, unsigned>::iterator end_it = map_data->strt_name_and_ID.lower_bound(street_prefix);
    
    // Traverse from start to end using start_id. Push the value of each pair traversed into the streetIDs vector.
    while(start_it != end_it){
        streetIDs.push_back(start_it->second);
        start_it++;
    }
    
    return streetIDs;
}

TurnType lookup_turn_type(unsigned street_segment_1, unsigned street_segment_2){
    auto it = map_data->segment_turns[street_segment_1].find(street_segment_2);
    if(it == map_data->segment_turns[street_segment_1].end())
        return TurnType::NONE;
    else
        return it->second;
}

std::vector<unsigned> find_intersection_ids_from_partial_intersection_name(std::string inter_prefix){
    inter_prefix = toLowerCase(inter_prefix); // convert inter_prefix to lower case so it works with the multimap and char manipulation.
    
    // Tried to check for commas and " and " but it's too hard :(
//    // User should be able to use commas and " and " to replace ampersand in intersection finding
//    if(inter_prefix.find(" , ") != std::string::npos)
//        inter_prefix.replace(inter_prefix.find(" , "), 3, " & ");
//    else if(inter_prefix.find(", ") != std::string::npos)
//        inter_prefix.replace(inter_prefix.find(", "), 2, " & ");
//    else if(inter_prefix.find(" and ") != std::string::npos)
//        inter_prefix.replace(inter_prefix.find(" and "), 5, " & ");

    std::vector<unsigned> interIDs; // The container to return
    
    // start iterator for the lower bound of the keys in inter_prefix range. Used to iterator over all street names.
    std::multimap<std::string, unsigned>::iterator start_it = map_data->inter_name_and_ID.lower_bound(inter_prefix);
    
    // Incrementing the last char in the string by 1 and lower_bound the iterator with the new string, which upper bounds the prefix names.
    inter_prefix[inter_prefix.length() - 1]++;
    // Assign end_it to the lower bound of the large value string/key, which results in a more reliable upper_bound. I.e. prefix <= key < prefix (last char + 1)
    std::multimap<std::string, unsigned>::iterator end_it = map_data->inter_name_and_ID.lower_bound(inter_prefix);
    
    // Traverse from start to end using start_id. Push the value of each pair traversed into the interIDs vector.
    while(start_it != end_it){
        interIDs.push_back(start_it->second);
        start_it++;
    }
    
    return interIDs;
}
