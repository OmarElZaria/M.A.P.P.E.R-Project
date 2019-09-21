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
 * March 3 2019
 */
#include "m3.h"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include <map>
#include <queue>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include "global.h"

#ifndef NO_VALUE
#define NO_VALUE -1
#endif

double y_lat_conversion(double lat);
double x_lon_conversion(double lon, double lat_point1, double lat_point2);
double cross_product(LatLon intersection, LatLon first_point, LatLon second_point);
double find_street_segment_travel_time_assistance(unsigned street_segment_id);
// BFS helper function
bool shortest_route_bfs(const unsigned intersect_id_start, const unsigned intersect_id_end, const double right_turn_penalty, const double left_turn_penalty);

// WaveItem constructor. Add comparing value fo A* algo later.
WaveItem::WaveItem(unsigned _intersectID, int _prevSegID, double _travelTime){
    intersectID = _intersectID;
    prevSegID = _prevSegID;
    travelTime = _travelTime;
}

//calculates cross product of two vectors
double cross_product(LatLon intersection, LatLon first_point, LatLon second_point){
    //parameters used to calculate cross product of segment vectors
    double x1, x2, y1, y2;
    
    x1 = x_lon_conversion(intersection.lon(), intersection.lat(), first_point.lat()) - x_lon_conversion(first_point.lon(), intersection.lat(), first_point.lat());
    y1 = y_lat_conversion(intersection.lat()) - y_lat_conversion(first_point.lat()); 
                   
    x2 = x_lon_conversion(second_point.lon(), intersection.lat(), second_point.lat()) - x_lon_conversion(intersection.lon(), intersection.lat(), second_point.lat());
    y2 = y_lat_conversion(second_point.lat()) - y_lat_conversion(intersection.lat());
    
    return (x1*y2 - x2*y1);
}

double find_street_segment_travel_time_assistance(unsigned street_segment_id){
   double length = find_street_segment_length(street_segment_id)/1000;        //getting the length of the street segment in km             
   
   InfoStreetSegment temp = getInfoStreetSegment(street_segment_id);             //getting the street segment info
   
   double time = (length/(temp.speedLimit))*3600;                                //finding the travel time in minutes
   
   return time;
}

double x_lon_conversion(double lon, double lat_point1, double lat_point2){
    double lat_avg = (((lat_point1 + lat_point2) * DEG_TO_RAD)/2);
    double x = lon * cos(lat_avg) * DEG_TO_RAD;
    return x;
}

double y_lat_conversion(double lat){
    double y = lat * DEG_TO_RAD;
    return y;
}

// Returns the turn type between two given segments.
// street_segment1 is the incoming segment and street_segment2 is the outgoing
// one.
// If the two street segments do not intersect, turn type is NONE.
// Otherwise if the two segments have the same street ID, turn type is 
// STRAIGHT.  
// If the two segments have different street ids, turn type is LEFT if 
// going from street_segment1 to street_segment2 involves a LEFT turn 
// and RIGHT otherwise.  Note that this means that even a 0-degree turn
// (same direction) is considered a RIGHT turn when the two street segments
// have different street IDs.
TurnType find_turn_type(unsigned street_segment1, unsigned street_segment2){
    //extracts information about the segments 
    InfoStreetSegment seg1 = getInfoStreetSegment(street_segment1);
    InfoStreetSegment seg2 = getInfoStreetSegment(street_segment2);

    //used to find position of segment end point
    LatLon seg1_end_position;
    LatLon seg2_end_position;
    //position of where seg1 and seg2 intersect
    LatLon common_intersection;   
   
    double crossProduct;
    
    //two segments have the same street ID, turn type is STRAIGHT.
    if(seg1.streetID == seg2.streetID)
        return TurnType::STRAIGHT;     
    
    //checks if the two segments have different street IDs
    if(seg1.streetID != seg2.streetID){
        
        //checks if end point of seg1 intersects with start point of seg 2
        if(seg1.to == seg2.from){
            common_intersection = getIntersectionPosition(seg1.to);
            
            //assigns end position of seg1 depending on number of curve points
            if(seg1.curvePointCount == 0){
                seg1_end_position = getIntersectionPosition(seg1.from);    
            }else{
                seg1_end_position = getStreetSegmentCurvePoint((seg1.curvePointCount)-1, street_segment1);
            }
            
            //assigns end position of seg2 depending on number of curve points
            if(seg2.curvePointCount == 0){
                seg2_end_position = getIntersectionPosition(seg2.to); 
            }else{
                seg2_end_position = getStreetSegmentCurvePoint(0, street_segment2);
            }
            
            crossProduct = cross_product(common_intersection, seg1_end_position, seg2_end_position);
            
            //positive cross product results in RIGHT turn, negative cross product results in LEFT turn 
            if(crossProduct < 0)
                return TurnType::RIGHT;
            if(crossProduct > 0)
                return TurnType::LEFT;
            if(crossProduct == 0)
                return TurnType::RIGHT;
            }
        
        //checks if start point of seg1 intersects with start point of seg 2
        if(seg1.from == seg2.from){
            common_intersection = getIntersectionPosition(seg1.from);
            
            //assigns end position of seg1 depending on number of curve points
            if(seg1.curvePointCount == 0){
               seg1_end_position = getIntersectionPosition(seg1.to);    
            }else{
               seg1_end_position = getStreetSegmentCurvePoint(0, street_segment1);
            }
            
            //assigns end position of seg2 depending on number of curve points
            if(seg2.curvePointCount == 0){
                seg2_end_position = getIntersectionPosition(seg2.to); 
            }else{
                seg2_end_position = getStreetSegmentCurvePoint(0, street_segment2);
            }
                    
            crossProduct = cross_product(common_intersection, seg1_end_position, seg2_end_position);
            
            //positive cross product results in LEFT turn, negative cross product results in RIGHT turn 
            if(crossProduct < 0)
                return TurnType::RIGHT;
            if(crossProduct > 0)
                return TurnType::LEFT;
            if(crossProduct == 0)
                return TurnType::RIGHT;
            }
        //checks if end point of seg1 intersects with end point of seg 2
        if(seg1.to == seg2.to){
            common_intersection = getIntersectionPosition(seg2.to);
            
            //assigns end position of seg1 depending on number of curve points
            if(seg1.curvePointCount == 0){
                seg1_end_position = getIntersectionPosition(seg1.from);    
            }else{
                seg1_end_position = getStreetSegmentCurvePoint((seg1.curvePointCount)-1, street_segment1);
            }
            
            //assigns end position of seg2 depending on number of curve points
            if(seg2.curvePointCount == 0){
                seg2_end_position = getIntersectionPosition(seg2.from); 
            }else{
                seg2_end_position = getStreetSegmentCurvePoint((seg2.curvePointCount)-1, street_segment2);
            }
            
            crossProduct = cross_product(common_intersection, seg2_end_position, seg1_end_position);
            
            //positive cross product results in RIGHT turn, negative cross product results in LEFT turn 
            if(crossProduct > 0)
                return TurnType::RIGHT;
            if(crossProduct < 0)
                return TurnType::LEFT;
            if(crossProduct == 0)
                return TurnType::RIGHT;
        }
        //checks if start point of seg1 intersects with end point of seg 2
        if(seg2.to == seg1.from){
            common_intersection = getIntersectionPosition(seg2.to);
            
            //assigns end position of seg1 depending on number of curve points
            if(seg1.curvePointCount == 0){
                seg1_end_position = getIntersectionPosition(seg1.to);    
            }else{
               seg1_end_position = getStreetSegmentCurvePoint(0, street_segment1);
            }
            
            //assigns end position of seg2 depending on number of curve points
            if(seg2.curvePointCount == 0){
                seg2_end_position = getIntersectionPosition(seg2.from); 
            }else{
                seg2_end_position = getStreetSegmentCurvePoint((seg2.curvePointCount)-1, street_segment2);
            }
                    
            crossProduct = cross_product(common_intersection, seg2_end_position, seg1_end_position);
            
            //positive cross product results in RIGHT turn, negative cross product results in LEFT turn 
            if(crossProduct > 0)
                return TurnType::RIGHT;
            if(crossProduct < 0)
                return TurnType::LEFT;
            if(crossProduct == 0)
                return TurnType::RIGHT;
        }
    }
    //return NONE if the two segments don't share an intersection
    return TurnType::NONE;
}

// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0.  The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given right_turn_penalty and left_turn_penalty (in seconds) per turn implied
// by the path.  If the turn type is STRAIGHT, then there is no penalty
double compute_path_travel_time(const std::vector<unsigned>& path, const double right_turn_penalty, const double left_turn_penalty){
    double total_travel_time = 0;
    TurnType type;
    
    //checks if the path vector is empty
    if(path.size() == 0){
        return total_travel_time;
    }
    
    //checks if the path vector has one element
    if(path.size() == 1){
        total_travel_time = find_street_segment_travel_time_assistance(path[0]);
        return total_travel_time;
    }
    
    
    //loop through the path vector
    for(unsigned i = 0; i < path.size(); i++){
       //travel time for each segment in the path
       
       total_travel_time += find_street_segment_travel_time_assistance(path[i]);
       
       //checks for turn and adds penalty to total time depending on turn type
       if(i < (path.size()-1)){
            type = find_turn_type(path[i], path[i+1]);
       
            if(type == TurnType::LEFT){
                total_travel_time += left_turn_penalty;
            }
            else if(type == TurnType::RIGHT){
                total_travel_time += right_turn_penalty;
            }
        }
    }
    
    return total_travel_time;
}

// Returns a path (route) between the start intersection and the end
// intersection, if one exists. This routine should return the shortest path
// between the given intersections, where the time penalties to turn right and
// left are given by right_turn_penalty and left_turn_penalty, respectively (in
// seconds).  If no path exists, this routine returns an empty (size == 0)
// vector.  If more than one path exists, the path with the shortest travel
// time is returned. The path is returned as a vector of street segment ids;
// traversing these street segments, in the returned order, would take one from
// the start to the end intersection.
std::vector<unsigned> find_path_between_intersections(const unsigned intersect_id_start, const unsigned intersect_id_end, const double right_turn_penalty, const double left_turn_penalty){
    // Reset intersection best times
    for(unsigned i = 0; i < intersections.size(); i++){
        intersections[i].bestTime = NO_VALUE;
    }
    
    std::vector<unsigned> path;
    // Search path and trace back route if a path was found.
    if(shortest_route_bfs(intersect_id_start, intersect_id_end, right_turn_penalty, left_turn_penalty))
        path = route_trace_back(intersect_id_end);
    
    return path; // path will either be empty or the trace back route
}

/**
 * Given a graph of intersections with a reachiingSegment parameter, build a vector of reachingSegments starting from a destination inter.
 * @param intersect_id_end
 * @return 
 */
std::vector<unsigned> route_trace_back(const unsigned intersect_id_end){
    // Get end intersection data and find reaching segment.
    std::vector<unsigned> path;
    intersection_data current_intersect = intersections[intersect_id_end];
    int prevSeg = current_intersect.reachingSegment;
    
    // Build path in reverse. Traverse through reachingSegments. Not sure why I cant have "reachingNodes".
    while(prevSeg != NO_VALUE && prevSeg >= 0){
        path.push_back(static_cast<unsigned>(prevSeg)); // push current segment
        
        // Determine the previous intersection and prep for next loop
        InfoStreetSegment temp = getInfoStreetSegment(static_cast<unsigned>(prevSeg));
        // Find intersection on other end of street segment. Don't need to check for oneWay
        if(static_cast<int>(current_intersect.id) == temp.to)
            current_intersect = intersections[temp.from];
        else
            current_intersect = intersections[temp.to];
        prevSeg = current_intersect.reachingSegment;
    }
    
    // Reorder path so first element is starting intersection, last element is dest intersection.
    // Needed since vectors don't have push_front capability.
    std::reverse(path.begin(), path.end());
    
    return path;
}

/**
 * Finds the shortest time path from start intersection to end intersection. Time is determined through segment travel time and right and left turn time penalties.
 * Returns true if a path is found, and sets reachingSegment flags in intersection_data. Returns false if no path found.
 * @param intersect_id_start
 * @param intersect_id_end
 * @param right_turn_penalty
 * @param left_turn_penalty
 * @return 
 */
bool shortest_route_bfs(const unsigned intersect_id_start, const unsigned intersect_id_end, const double right_turn_penalty, const double left_turn_penalty){
    // Make a reversed max heap using WaveItemComparator, containing WaveItems
    std::priority_queue<WaveItem, std::vector<WaveItem>, WaveItemComparator> wavefront;
    
    wavefront.emplace(intersect_id_start, NO_VALUE, 0.0); // starting node with NO_VALUE for no prev seg and 0.0s to reach it.
    
    while(wavefront.size() > 0){
        // get the element at the top of the heap
        WaveItem wave = wavefront.top();
        wavefront.pop();
        intersection_data* currIntersect = &intersections[wave.intersectID];
        // limit re-expansion if this node has been reached quicker than before
        // Paths are currently able to loop back towards the start but shouldnt expand to it again due to this condition.
        if(currIntersect->bestTime == NO_VALUE || wave.travelTime < currIntersect->bestTime){
            // New best time to node? Update reaching segment and best time,
            currIntersect->reachingSegment = wave.prevSegID;
            currIntersect->bestTime = wave.travelTime;
            
            // check if i got to the end
            if(currIntersect->id == intersect_id_end)
                return true;
            
            std::vector<unsigned> outSegments = find_intersection_street_segments(currIntersect->id);
            
             // Push child intersections into the heap
            //
            for(unsigned i = 0; i < outSegments.size(); i++){
                if(static_cast<int>(outSegments[i]) != currIntersect->reachingSegment){
                    // Determine which way we are traveling down the segment and add the dest intersect.
                    InfoStreetSegment temp = getInfoStreetSegment(outSegments[i]);
                    
                    if(temp.from == static_cast<int>(currIntersect->id)){
                        // add temp.to to the wavefront. find_turn_time should always produce a valid time here.
                        double turn_time = 0.0;
                        if(currIntersect->reachingSegment != NO_VALUE)
                            turn_time = find_turn_time(currIntersect->reachingSegment, outSegments[i], right_turn_penalty, left_turn_penalty);
                        
                        wave = WaveItem(
                                  temp.to,
                                  outSegments[i],
                                  currIntersect->bestTime
                                + find_street_segment_travel_time(outSegments[i]) 
                                + turn_time
                        );
                        wavefront.push(wave);
                    }else if(!temp.oneWay && temp.to == static_cast<int>(currIntersect->id)){
                        // add temp.from to the wavefront. find_turn_time should always produce a valid time here.
                        double turn_time = 0.0;
                        if(currIntersect->reachingSegment != NO_VALUE)
                            turn_time = find_turn_time(currIntersect->reachingSegment, outSegments[i], right_turn_penalty, left_turn_penalty);
                        
                        wave = WaveItem(
                                  temp.from,
                                  outSegments[i],
                                  currIntersect->bestTime
                                + find_street_segment_travel_time(outSegments[i]) 
                                + turn_time
                        );
                        wavefront.push(wave);
                    }
                }
            }
        }
    }
    
    return false;
}

/**
 * Travelling from seg 1 to seg 2, determine time to turn. Returns NO_VALUE if turn type found is NONE, returns turn time in seconds otherwise.
 * @param street_segment_1
 * @param street_segment_2
 * @return 
 */
double find_turn_time(const unsigned street_segment_1, const unsigned street_segment_2, const double right_turn_penalty, const double left_turn_penalty){
    TurnType turn = lookup_turn_type(street_segment_1, street_segment_2);
    if(turn == TurnType::RIGHT)
        return right_turn_penalty;
    else if(turn == TurnType::STRAIGHT)
        return 0.0;
    else if(turn == TurnType::LEFT)
        return left_turn_penalty;
    else
        return NO_VALUE;
}

