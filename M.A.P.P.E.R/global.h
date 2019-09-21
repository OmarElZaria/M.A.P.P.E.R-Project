/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   global.h
 * Author: juraseks
 *
 * Created on February 25, 2019, 7:08 PM
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "StreetsDatabaseAPI.h"
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include "m3.h"

#define NO_VALUE -1

struct map_data_structures {
    std::vector<std::vector<unsigned>> inter_strt_segs_list; // intersection ID by street segment ID; Outer vector sorted by inter, nested vector has segments @ inter
    std::vector<std::vector<unsigned>> strt_segs_ID_list; // street ID and street segment ID; all street segment ids for the corresponding street id
    std::vector<std::vector<unsigned>> list_strt_intersections; //holds all street intersections for the corresponding street id
    std::vector<double> strt_seg_travel_time; // holds travel time for the corresponding street segment Id
    std::multimap<std::string, unsigned> strt_name_and_ID; // multimap using pair of street name and street ID in order to identify streets by name
    std::multimap<std::string, unsigned> inter_name_and_ID;
    std::vector<std::unordered_set<unsigned>> hash_strt_intersections; // vector indexed by street id containing hash tables using intersection ids as keys
    std::vector<std::unordered_map<unsigned, TurnType>> segment_turns; // Indexed by segments making turns with neighbouring segments. Doesn't include non-neighbouring segments.
};

// UI intersection data
struct intersection_data{
    unsigned id;
    LatLon position;
    std::string name;
    bool highlight = false;
    int reachingSegment = NO_VALUE;
    double bestTime = NO_VALUE;
};

// UI POI data
struct POI_data{
    LatLon position;
    std::string name;
    std::string type;
    bool highlight = false;
};

// UI street seg data
struct street_segment_data{
    unsigned id;
    LatLon start_position;
    LatLon end_position;
    double length;
    float speedlimit;
    int curvePointCount;
    bool highlight = false;
    std::string street_name;
    bool oneWay;
};

extern std::vector<intersection_data> intersections; // hold every intersection's data
extern std::vector<POI_data> POIs; // hold all POI data
extern std::vector<Feature> features; // hold every feature
extern std::vector<street_segment_data> street_segments; // hold all street segment data

extern double max_lat;   //initializing min and max values
extern double min_lat;
extern double max_lon;
extern double min_lon;

extern bool zoom_level_end; // bool to signal when breaking point of zooming has been passed

extern bool filter_streets;
extern bool filter_intersections;

extern int start_id;

// Accessor for segment_turns data structure. Needs to be implemented in m1.cpp
TurnType lookup_turn_type(unsigned street_segment_1, unsigned street_segment_2);

// Route retracing helper function
std::vector<unsigned>   route_trace_back(const unsigned intersect_id_end);

std::vector<unsigned> find_intersection_ids_from_partial_intersection_name(std::string inter_prefix);

// Turn time helper function
double find_turn_time(const unsigned street_segment_1, const unsigned street_segment_2, const double right_turn_penalty, const double left_turn_penalty);

// Wavefront item struct for bfs search
struct WaveItem{
    unsigned intersectID;
    int prevSegID;
    double travelTime;
//    double distanceToTarget;
    WaveItem(unsigned _intersectID, int _prevSegID, double _travelTime);
};

// class to compare two WaveItems by travelTime. Edit comparing value for A* algo.
class WaveItemComparator{
public:
    int operator() (const WaveItem & w1, const WaveItem & w2){
        return w1.travelTime > w2.travelTime;
    }
};

#endif /* GLOBAL_H */

