
/* ECE297 
 *cd 092
 *Milestone 4
 *TRUCK mapping  
 * 
 Abbas Majed, Omar, Scott, Neehar
 edited : 3/30/2019 at 12:20 am

*/
        
#include "m4.h"
#include "m3.h"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include <vector>
#include <queue>
#include <list>
#include "global.h"
#include <cctype>
#include <algorithm>
#include <chrono>
#define TIME_LIMIT 45

#ifndef NO_VALUE
#define NO_VALUE -1
#endif

bool deliveryComparator (const std::pair<double, unsigned> & p1, const std::pair<double, unsigned> & p2);
bool multiStartComparator (const std::pair<double, std::pair<unsigned,unsigned>> & p1, const std::pair<double, std::pair<unsigned,unsigned>> & p2);

bool deliveryComparator (const std::pair<double, unsigned> & p1, const std::pair<double, unsigned> & p2) {
    return p1.first < p2.first; 
}

bool multiStartComparator (const std::pair<double, std::pair<unsigned,unsigned>> & p1, const std::pair<double, std::pair<unsigned,unsigned>> & p2) {
    return p1.first < p2.first;
}

enum class NodeType {
    DEPOT,
    PICK_UP,
    DROP_OFF,
    BOTH
};

struct route_data_structure{
    // hash map of min heaps sorted by path time indexed by intersection ID1s
    // min heaps are vectors of pairs of a travel time and a pair of intersection ID2s and path from ID1 to ID2
    std::vector< std::pair<double, std::pair<unsigned, unsigned>> > multi_start_paths; //keep track of the shortest depo to pickup  
    std::unordered_map<unsigned, std::vector<std::pair<double, unsigned>> > delivery_to_delivery; // Nested hash map indexed by delivery IDs to other deliveries IDs, producing a travel time double.
    std::unordered_map<unsigned, std::vector<std::pair<double, unsigned>> > dropOff_to_depot; // Nested hash map indexed by delivery IDs to other deliveries IDs, producing a travel time double.
    std::unordered_map<unsigned, std::unordered_map<unsigned, std::vector<unsigned>> > delivery_and_depo_graph; // Nested hash map indexed by delivery IDs to other deliveries IDs, producing a travel time double.
    std::unordered_map<unsigned, NodeType> node_type_hashmap; // determine what kind of node is at an intersection to prevent going over the same intersection twice
    std::vector<unsigned> depots;
};

route_data_structure* route_data = NULL;

bool bfs_multi_dest_helper(const unsigned intersect_id_start, std::unordered_set<unsigned> & intersect_id_end, const double right_turn_penalty, const double left_turn_penalty);
bool multi_dest_search(const unsigned intersect_id_start, const std::vector<unsigned> & pickUp_destinations, const std::vector<unsigned> & dropOff_destinations, 
        const std::vector<unsigned> & depot_destinations, const double right_turn_penalty, const double left_turn_penalty);

std::vector<CourierSubpath> traveling_courier(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots, 
const float right_turn_penalty, const float left_turn_penalty, const float truck_capacity){
    
    auto startTime = std::chrono::high_resolution_clock::now();
    bool timeOut = false;
    
    unsigned N = deliveries.size();
    unsigned M = depots.size();
    std::vector<CourierSubpath> final_path;
    std::unordered_map<unsigned, std::vector<std::pair<double, unsigned>> > deliveryIndex; // hold delivery ID(s) of the delivery locations at the key intersection ID
    
    // Allocate route data structure
    route_data = new route_data_structure;
    route_data->depots = depots;
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // EVERYTHING PERTAINING TO ROUTE DATA STRUCTURE LOADING. MAY INCLUDE SOME LOCAL VARIABLES THAT ARE USED LATER IN THE FUNCTION //
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    std::vector<unsigned> unique_nodes; // vector of unique intersection IDs to be loaded into the route data structures
    std::unordered_set<unsigned> pickUp_hash;
    std::unordered_set<unsigned> dropOff_hash;
    
    // Load NodeType data structure with depots
    for(unsigned i = 0; i < M; i++){
        route_data->node_type_hashmap[depots[i]] = NodeType::DEPOT;
        unique_nodes.push_back(depots[i]);
    }
    // Load NodeType data structure with deliveries
    for(unsigned i = 0; i < N; i++){
        // check pickUp node of the delivery
        if(route_data->node_type_hashmap.find(deliveries[i].pickUp) == route_data->node_type_hashmap.end()){
            // If no node type exists for this intersections, update it.
            route_data->node_type_hashmap[deliveries[i].pickUp] = NodeType::PICK_UP;
            unique_nodes.push_back(deliveries[i].pickUp);
            
            // Update deliveryIndex since it conditionally coincides with node_type_hash map
            std::vector<std::pair<double, unsigned>> temp;
            deliveryIndex[deliveries[i].pickUp] = temp;
        }else if(route_data->node_type_hashmap[deliveries[i].pickUp] == NodeType::DROP_OFF){
            // If a node type does exist for this intersection, check if it's an alternate form and update it, otherwise no need to change it.
            route_data->node_type_hashmap[deliveries[i].pickUp] = NodeType::BOTH;
        }
        pickUp_hash.insert(deliveries[i].pickUp);
        deliveryIndex[deliveries[i].pickUp].push_back({deliveries[i].itemWeight, i});
        
        // check dropOff node of the delivery
        if(route_data->node_type_hashmap.find(deliveries[i].dropOff) == route_data->node_type_hashmap.end()){
            // If no node type exists for this intersections, update it.
            route_data->node_type_hashmap[deliveries[i].dropOff] = NodeType::DROP_OFF;
            unique_nodes.push_back(deliveries[i].dropOff);
            
            // Update deliveryIndex since it conditionally coincides with node_type_hash map
            std::vector<std::pair<double, unsigned>> temp;
            deliveryIndex[deliveries[i].dropOff] = temp;
        }else if(route_data->node_type_hashmap[deliveries[i].dropOff] == NodeType::PICK_UP){
            // If a node type does exist for this intersection, check if it's an alternate form and update it, otherwise no need to change it.
            route_data->node_type_hashmap[deliveries[i].dropOff] = NodeType::BOTH;
        }
        dropOff_hash.insert(deliveries[i].dropOff);
        deliveryIndex[deliveries[i].dropOff].push_back({deliveries[i].itemWeight, i});
        
        // If the truck can't handle a delivery then return empty path
        if(deliveries[i].itemWeight > truck_capacity){
            delete route_data;
            return final_path;
        }
    }
        
    std::vector<unsigned> pickUp_destinations;
    pickUp_destinations.insert(pickUp_destinations.begin(), pickUp_hash.begin(), pickUp_hash.end());
    
    std::vector<unsigned> dropOff_destinations;
    dropOff_destinations.insert(dropOff_destinations.begin(), dropOff_hash.begin(), dropOff_hash.end());
    
    // Sort deliveries
    for(auto it = deliveryIndex.begin(); it != deliveryIndex.end(); it++){
        std::sort(it->second.begin(), it->second.end(), deliveryComparator);
    }
    
    // Pre-load all relevant paths
    for(unsigned i=0; i < unique_nodes.size(); i++){
        if(!multi_dest_search(unique_nodes[i], pickUp_destinations, dropOff_destinations, depots, right_turn_penalty, left_turn_penalty)){
            delete route_data;
            return final_path;
        }else if(route_data->node_type_hashmap[unique_nodes[i]] != NodeType::DEPOT){
            if(route_data->node_type_hashmap[unique_nodes[i]] != NodeType::PICK_UP)
                std::sort(route_data->dropOff_to_depot[unique_nodes[i]].begin(), route_data->dropOff_to_depot[unique_nodes[i]].end(), deliveryComparator);
            std::sort(route_data->delivery_to_delivery[unique_nodes[i]].begin(), route_data->delivery_to_delivery[unique_nodes[i]].end(), deliveryComparator);
        }
    }
    
    // Global vector sorting (only multi_start_paths)
    std::sort(route_data->multi_start_paths.begin(), route_data->multi_start_paths.end(), multiStartComparator);
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////   GREEDY ($$$) ALGORITHM!   /////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    double final_time = NO_VALUE;
    int visitCount = 0;
    
    for(unsigned multi_index = 0; !timeOut && multi_index < route_data->multi_start_paths.size(); multi_index++){
        
        unsigned multi_2_limit_loc = route_data->multi_start_paths[multi_index].second.second;
        unsigned multi_2_limit = route_data->delivery_to_delivery[multi_2_limit_loc].size();
        
        for(unsigned multi_2 = 0; !timeOut && multi_2 < multi_2_limit; multi_2++){
            
//            unsigned multi_3_limit = multi_2_limit;
//            
//            for(unsigned multi_3 = 0; !timeOut && multi_3 < multi_3_limit; multi_3++){
//                for(unsigned multi_4 = 0; !timeOut && multi_4 < multi_3_limit; multi_4++){
                    visitCount = 0;
                    std::vector<CourierSubpath> particular_path;

                    std::unordered_set<unsigned> remainingDeliveries;
                    std::unordered_set<unsigned> currentPackages; // hold current delivery ID
                    float truck_weight = 0.0;

                    for(unsigned i = 0; i < N; i++){
                        remainingDeliveries.insert(i);
                    }

                    // Go from first depot to first package pick up
                    unsigned first_start = route_data->multi_start_paths[multi_index].second.first;
                    unsigned first_end = route_data->multi_start_paths[multi_index].second.second;
                    std::vector<unsigned> first_path = route_data->delivery_and_depo_graph[first_start][first_end];

                    std::vector<unsigned> first_pickUp;
                    particular_path.push_back({first_start, first_end, first_path, first_pickUp});

                    std::vector<unsigned> keyDeliveries;
                    if(true){
                        float temp_weight = truck_weight;
                        for(unsigned i=0; i < deliveryIndex[first_end].size(); i++){
                            // If the pickup has not been visited and can fit inside the truck, pick up the package
                            if(deliveries[deliveryIndex[first_end][i].second].pickUp == first_end && deliveries[deliveryIndex[first_end][i].second].itemWeight + temp_weight <= truck_capacity){
                                temp_weight += deliveries[deliveryIndex[first_end][i].second].itemWeight;
                                keyDeliveries.push_back(deliveryIndex[first_end][i].second);
                            }
                        }
                    }

                    // utilize remainingDeliveries, currentPackages, deliveryIndex, deliveries, node_type_hash map, delivery_to_delivery to make this legal. delivery_and_depot_graph for speed.
                    unsigned temp_start = first_end;
                    unsigned temp_end = NO_VALUE;
                    std::vector<unsigned> pickUp_indices;
                    while(remainingDeliveries.size() > 0){
                        pickUp_indices.clear();
                        visitCount++;
                        // some logic to figure out what to do with the start node
                        if(route_data->node_type_hashmap[temp_start] == NodeType::PICK_UP){
                            for(unsigned i=0; i < keyDeliveries.size(); i++){
                                // If the pickup has not been visited and can fit inside the truck, pick up the package
                                truck_weight += deliveries[keyDeliveries[i]].itemWeight;
                                currentPackages.insert(keyDeliveries[i]);
                                pickUp_indices.push_back(keyDeliveries[i]);
                            }
                        }else if(route_data->node_type_hashmap[temp_start] == NodeType::DROP_OFF){
                            for(unsigned i=0; i < keyDeliveries.size(); i++){
                                // If the delivery dropoff has not been visited and I currently have the package for it, drop off the package
                                truck_weight -= deliveries[keyDeliveries[i]].itemWeight;
                                currentPackages.erase(keyDeliveries[i]);
                                remainingDeliveries.erase(keyDeliveries[i]);
                            }
                        }else if(route_data->node_type_hashmap[temp_start] == NodeType::BOTH){
                            for(unsigned i=0; i < keyDeliveries.size(); i++){
                                // If the delivery dropoff has not been visited and I currently have the package for it, drop off the package
                                if(deliveries[keyDeliveries[i]].pickUp == temp_start){
                                    truck_weight += deliveries[keyDeliveries[i]].itemWeight;
                                    currentPackages.insert(keyDeliveries[i]);
                                    pickUp_indices.push_back(keyDeliveries[i]);
                                }else if(deliveries[keyDeliveries[i]].dropOff == temp_start){
                                    truck_weight -= deliveries[keyDeliveries[i]].itemWeight;
                                    currentPackages.erase(keyDeliveries[i]);
                                    remainingDeliveries.erase(keyDeliveries[i]);
                                }
                            }
                        }

                        // I should only go to a dropoff if I have the package and I should only go to a pickup if I have space
                        bool isLegal = false;
                        keyDeliveries.clear();

                        if(remainingDeliveries.size() > 0 ){
                            unsigned i=0;
                            if(visitCount == 1)
                                i = multi_2;
//                            else if(visitCount == 2)
//                                i = multi_3;
//                            else if(visitCount == 3)
//                                i = multi_4;
                            for(; !isLegal && i < route_data->delivery_to_delivery[temp_start].size(); i++){
                                unsigned to_node = route_data->delivery_to_delivery[temp_start][i].second;
                                if(route_data->node_type_hashmap[to_node] == NodeType::PICK_UP){
                                    float temp_weight = truck_weight;
                                    for(unsigned j=0; j < deliveryIndex[to_node].size(); j++){
                                        // If the pickup has not been visited and can fit inside the truck, pick up the package
                                        if(remainingDeliveries.find(deliveryIndex[to_node][j].second) != remainingDeliveries.end() && currentPackages.find(deliveryIndex[to_node][j].second) == currentPackages.end()
                                                && deliveries[deliveryIndex[to_node][j].second].itemWeight + temp_weight <= truck_capacity){
                                            temp_weight += deliveries[deliveryIndex[to_node][j].second].itemWeight;
                                            isLegal = true;
                                            keyDeliveries.push_back(deliveryIndex[to_node][j].second);
                                        }
                                    }
                                }else if(route_data->node_type_hashmap[to_node] == NodeType::DROP_OFF){
                                    for(unsigned j=0; j < deliveryIndex[to_node].size(); j++){
                                        // If the delivery dropOff has not been visited and I currently have the package for it, drop off the package
                                        if(remainingDeliveries.find(deliveryIndex[to_node][j].second) != remainingDeliveries.end() && currentPackages.find(deliveryIndex[to_node][j].second) != currentPackages.end()){
                                            keyDeliveries.push_back(deliveryIndex[to_node][j].second);
                                            isLegal = true;
                                        }
                                    }
                                }else if(route_data->node_type_hashmap[to_node] == NodeType::BOTH){
                                    float temp_weight = truck_weight;
                                    for(unsigned j=0; j < deliveryIndex[to_node].size(); j++){
                                        // If the delivery dropOff has not been visited and I currently have the package for it, drop off the package
                                        // TODO Check for drop offs before pickups!!!
                                        if(deliveries[deliveryIndex[to_node][j].second].dropOff == to_node){
                                            if(remainingDeliveries.find(deliveryIndex[to_node][j].second) != remainingDeliveries.end() && currentPackages.find(deliveryIndex[to_node][j].second) != currentPackages.end()){
                                                temp_weight -= deliveries[deliveryIndex[to_node][j].second].itemWeight;
                                                keyDeliveries.push_back(deliveryIndex[to_node][j].second);
                                                isLegal = true;
                                            }
                                        }
                                    }
                                    for(unsigned j=0; j < deliveryIndex[to_node].size(); j++){
                                        // If the delivery dropOff has not been visited and I currently have the package for it, drop off the package
                                        // TODO Check for drop offs before pickups!!!
                                        if(deliveries[deliveryIndex[to_node][j].second].pickUp == to_node){
                                            if(remainingDeliveries.find(deliveryIndex[to_node][j].second) != remainingDeliveries.end() && currentPackages.find(deliveryIndex[to_node][j].second) == currentPackages.end()
                                                    && deliveries[deliveryIndex[to_node][j].second].itemWeight + temp_weight <= truck_capacity){
                                                temp_weight += deliveries[deliveryIndex[to_node][j].second].itemWeight;
                                                keyDeliveries.push_back(deliveryIndex[to_node][j].second);
                                                isLegal = true;
                                            }
                                        }
                                    }
                                }
                                temp_end = to_node;
                            }
                        }
                        if(remainingDeliveries.size() > 0){
                            std::vector<unsigned> temp_path = route_data->delivery_and_depo_graph[temp_start][temp_end];
                            particular_path.push_back({temp_start, temp_end, temp_path, pickUp_indices});
                            temp_start = temp_end;
                            temp_end = 0;
                        }
                    }
                    // Travel to closest depot
                    unsigned close_depot = route_data->dropOff_to_depot[temp_start][0].second;
                    pickUp_indices.clear();
                    std::vector<unsigned> last_path = route_data->delivery_and_depo_graph[temp_start][close_depot];
                    particular_path.push_back({temp_start, close_depot, last_path, pickUp_indices});

                    // Update final_path if we need to
                    double temp_time = 0.0;
                    for(unsigned i=0; i < particular_path.size(); i++){
                        temp_time += compute_path_travel_time( particular_path[i].subpath, right_turn_penalty, left_turn_penalty);
                    }
                    if(final_time == NO_VALUE || temp_time < final_time){
                        final_time = temp_time;
                        final_path = particular_path;
                    }
                    
                    // Some clock shit
                    auto currentTime = std::chrono::high_resolution_clock::now();
                    auto wallClock = std::chrono::duration_cast<std::chrono::duration<double>> (currentTime - startTime);

                    // Keep optimizing until within 2% of time limit
                    if (wallClock.count() > 0.98 * TIME_LIMIT)
                        timeOut = true;
//                }
//            }
        }
    }
    
//    Go from any depot to nearest pickup, p
//    while (packages to deliver)
//    p = nearest legal pickUp or dropOff
//    solution += path to p
//    }
//    Go to nearest depot
    
//    // Complete deliveries, in order
//    for(unsigned i = 0; i < N - 1; i++){
//        unsigned temp_start = deliveries[i].pickUp;
//        unsigned temp_end = deliveries[i].dropOff;
//        std::vector<unsigned> temp_path = route_data->delivery_and_depo_graph[deliveries[i].pickUp][deliveries[i].dropOff];
//        unsigned temp_pickUp_index = i; 
//        std::vector<unsigned> temp_pickUp;
//        temp_pickUp.push_back(temp_pickUp_index);
//        final_path.push_back({temp_start, temp_end, temp_path, temp_pickUp});
//        
//        unsigned next_temp_start = deliveries[i].dropOff;
//        unsigned next_temp_end = deliveries[i + 1].pickUp;
//        std::vector<unsigned> next_temp_path = route_data->delivery_and_depo_graph[deliveries[i].dropOff][deliveries[i + 1].pickUp];
//        std::vector<unsigned> next_temp_pickUp;
//        final_path.push_back({next_temp_start, next_temp_end, next_temp_path, next_temp_pickUp});
//    }
//    
//    // Drop off last package
//    unsigned second_last_start = deliveries[N - 1].pickUp;
//    unsigned second_last_end = deliveries[N - 1].dropOff;
//    std::vector<unsigned> second_last_path = route_data->delivery_and_depo_graph[second_last_start][second_last_end];
//    unsigned last_pickUp_index = N - 1; 
//    std::vector<unsigned> last_pickUp;
//    last_pickUp.push_back(last_pickUp_index);
//    final_path.push_back({second_last_start, second_last_end, second_last_path, last_pickUp});
//    
//    // Go back to the first depot to drop off the truck
//    unsigned last_start = deliveries[N - 1].dropOff;
//    unsigned last_end = route_data->dropOff_to_depot[last_start][0].second;
//    std::vector<unsigned> last_path = route_data->delivery_and_depo_graph[last_start][last_end];
//    std::vector<unsigned> no_pickUp;
//    final_path.push_back({last_start, last_end, last_path, no_pickUp});
    
    delete route_data;
    
    return final_path;
}

// Return true if success, false if this problem cannot be completed.
bool multi_dest_search(const unsigned intersect_id_start, const std::vector<unsigned> & pickUp_destinations, const std::vector<unsigned> & dropOff_destinations, 
        const std::vector<unsigned> & depot_destinations, const double right_turn_penalty, const double left_turn_penalty){
    // Reset intersection best times
    for(unsigned i = 0; i < intersections.size(); i++){
        intersections[i].bestTime = NO_VALUE;
        intersections[i].reachingSegment = NO_VALUE; // useful for debugging purposes
    }
//    intersections[intersect_id_start].reachingSegment = NO_VALUE;
    
    NodeType start_type = route_data->node_type_hashmap[intersect_id_start];
    
    // Build destinations hash table to pass into the bfs helper
    std::unordered_set<unsigned> destinations(pickUp_destinations.begin(), pickUp_destinations.end()); // all node types will search for pick ups
    if(start_type != NodeType::DEPOT) // depots don't want to search for drop offs
        destinations.insert(dropOff_destinations.begin(), dropOff_destinations.end());
    if(start_type == NodeType::DROP_OFF || start_type == NodeType::BOTH) // only drop offs want to search for depots
        destinations.insert(depot_destinations.begin(), depot_destinations.end());
    
    // If destinations contains the starting intersection, erase it from the hash table to prevent issues.
    // Typically will use this helper function with the starting intersection included
    if(destinations.find(intersect_id_start) != destinations.end())
        destinations.erase(intersect_id_start);
    
    // Do multi dest bfs on delivery destinations plus depot destinations at intersection start
    if(!bfs_multi_dest_helper(intersect_id_start, destinations, right_turn_penalty, left_turn_penalty)){
        // if bfs fails, then check if I was looking for depots and ensure that it only failed on a depot search.
        if(start_type == NodeType::DROP_OFF || start_type == NodeType::BOTH){ // BFS will operate a depot search if the node type contains drop_off
            unsigned bad_depots = 0;
            for(unsigned i = 0; i < route_data->depots.size(); i++){
                if(destinations.find(route_data->depots[i]) != destinations.end())
                    bad_depots++;
            }
            // if there are inaccessible deliveries or not enough available depots, return false
            if(bad_depots < destinations.size() || bad_depots == route_data->depots.size())
                return false;
        }else if(start_type == NodeType::PICK_UP) // pick up nodes don't search for depots, therefore no solution if bfs fails here.
            return false;
        // Depots are allowed to fail path finding.
    }
    
    // If we successfully did bfs then we can add to our data structures.
    if(start_type != NodeType::DEPOT && route_data->delivery_to_delivery.find(intersect_id_start) == route_data->delivery_to_delivery.end()){
        std::vector<std::pair<double, unsigned>> temp;
        route_data->delivery_to_delivery[intersect_id_start] = temp;
    }
    if((start_type == NodeType::DROP_OFF || start_type == NodeType::BOTH) && route_data->dropOff_to_depot.find(intersect_id_start) == route_data->dropOff_to_depot.end()){
        std::vector<std::pair<double, unsigned>> temp;
        route_data->dropOff_to_depot[intersect_id_start] = temp;
    }
    if(route_data->delivery_and_depo_graph.find(intersect_id_start) == route_data->delivery_and_depo_graph.end()){
        std::unordered_map<unsigned, std::vector<unsigned>> temp;
        route_data->delivery_and_depo_graph[intersect_id_start] = temp;
    }
    
    // Retrace to PICK UP destinations (regardless of node type)
    for(unsigned i=0; i<pickUp_destinations.size(); i++){
        // If the code reaches here with an invalid path then something might be wrong
        std::vector<unsigned> path = route_trace_back(pickUp_destinations[i]);
        double path_time = compute_path_travel_time(path, right_turn_penalty, left_turn_penalty);

        // Build global data structures (add to multi_start_paths or delivery_to_delivery; and delivery_and_depot_graph))
        if(start_type == NodeType::DEPOT && path.size() != 0)
            route_data->multi_start_paths.push_back({ path_time, {intersect_id_start, pickUp_destinations[i]} });
        else 
            route_data->delivery_to_delivery[intersect_id_start].push_back( {path_time, pickUp_destinations[i]} );
        
        route_data->delivery_and_depo_graph[intersect_id_start][pickUp_destinations[i]] = path;
    }
    
    // Retrace to DROP OFF destinations (if the starting node is NOT a DEPOT)
    if(start_type != NodeType::DEPOT){
        for(unsigned i=0; i<dropOff_destinations.size(); i++){
            std::vector<unsigned> path = route_trace_back(dropOff_destinations[i]);
            double path_time = compute_path_travel_time(path, right_turn_penalty, left_turn_penalty);

            // Build global data structures
            route_data->delivery_to_delivery[intersect_id_start].push_back( {path_time, dropOff_destinations[i]} );
            route_data->delivery_and_depo_graph[intersect_id_start][dropOff_destinations[i]] = path;
        }
    }
    
    // Retrace to DEPOT destinations (if and only if the starting node is a DROP OFF)
    if(start_type == NodeType::DROP_OFF || start_type == NodeType::BOTH){
        for(unsigned i=0; i<depot_destinations.size(); i++){
            // Ensure that the current depot id is accessible via path
            if(destinations.find(depot_destinations[i]) == destinations.end()){
                std::vector<unsigned> path = route_trace_back(depot_destinations[i]);
                double path_time = compute_path_travel_time(path, right_turn_penalty, left_turn_penalty);

                // Build global data structures
                route_data->dropOff_to_depot[intersect_id_start].push_back( {path_time, depot_destinations[i]} );
                route_data->delivery_and_depo_graph[intersect_id_start][depot_destinations[i]] = path;
            }
        }
    }
    
    return true;
}

bool bfs_multi_dest_helper(const unsigned intersect_id_start, std::unordered_set<unsigned> & intersect_id_end, const double right_turn_penalty, const double left_turn_penalty){
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
            if(intersect_id_end.find(currIntersect->id) != intersect_id_end.end())
                intersect_id_end.erase(currIntersect->id);
            if(intersect_id_end.size() == 0)
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


/*
#include <map>
#include <queue>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include "global.h"

std::vector<unsigned> graph_deliveries(std::vector<DeliveryInfo> deliveries);
unsigned find_nearest_depot(unsigned intersection_id, const std::vector<unsigned>& depots);

struct mystruct{
    std::vector<unsigned> subpath;
    
    double distance;
    
    //unsigned start_intersection;
    
    //unsigned end_intersection;
};

std::vector<CourierSubpath> traveling_courier(
		const std::vector<DeliveryInfo>& deliveries,
	       	const std::vector<unsigned>& depots, 
		const float right_turn_penalty, 
		const float left_turn_penalty, 
		const float truck_capacity){
    
    int N = deliveries.size();
    
    std::vector<mystruct> temp;
    std::vector<std::vector <mystruct> > adj_matrix;  
    std::vector<bool> visited;
    std::vector<double> shortest_distance;
    
    std::vector<unsigned> nodes = graph_deliveries(deliveries);
    
    std::vector<CourierSubpath> final_path;
    std::vector<CourierSubpath> empty_path;
    
    // Go from first depot to first package pick up
    unsigned start = find_nearest_depot(nodes[0], depots);
    unsigned end = nodes[0];
    std::vector<unsigned> path = find_path_between_intersections(start, end, right_turn_penalty, left_turn_penalty);
    
    
    
    std::vector<unsigned> pickUp_indices;
    std::vector<unsigned> drop_offs;
    final_path.push_back({start, end, path, pickUp_indices});
    
   /* if(nodes.size() == 2){
        start = nodes[0];
        end = nodes[1];
        path = find_path_between_intersections(start, end, right_turn_penalty, left_turn_penalty);
        pickUp_indices.push_back(0);
        
        final_path.push_back({start, end, path, pickUp_indices});
        
        start = nodes[1];
        end = find_nearest_depot(nodes[1], depots);
        path = find_path_between_intersections(start, end, right_turn_penalty, left_turn_penalty);
        
        final_path.push_back({start, end, path, drop_offs});
        
        return final_path;
    }
    
    double distance = find_distance_between_two_points(getIntersectionPosition(nodes[0]), getIntersectionPosition(nodes[1]));
    mystruct matrix_contents;
    for(int i=0; i < 2*N - 1; i++){    
        for(int j = 0; j < 2*N - 1; j++){
            matrix_contents.distance = find_distance_between_two_points(getIntersectionPosition(nodes[i]), getIntersectionPosition(nodes[j]));
            
            matrix_contents.subpath = find_path_between_intersections(nodes[i], nodes[j], right_turn_penalty, left_turn_penalty);
           // matrix_contents.start_intersection = nodes[i];
           // matrix_contents.end_intersection = nodes[j];
            temp.push_back({matrix_contents.subpath, matrix_contents.distance, matrix_contents.start_intersection = nodes[i], matrix_contents.end_intersection});         
            if(j%2 == 0 && (i!=j) && matrix_contents.distance <= distance){
                distance = matrix_contents.distance;
            }
        }
        visited.push_back(false);
        shortest_distance.push_back(distance);
        adj_matrix.push_back(temp);
        temp.clear();
    }
    
    int starting_point = 0;
    int final_index = 0;
    int ending_point;
    double weight = 0;
    //double shortest_distance = adj_matrix[0][1].distance;
    double current_distance = 0;
    
    for(ending_point = 0; ending_point < 2*N - 1; ending_point++){
        if(ending_point%2 == 0  && (weight + deliveries[ending_point/2].itemWeight) < truck_capacity && !visited[ending_point]){
            //std::cout<<"hello"<<std::endl;
            current_distance = adj_matrix[starting_point][ending_point].distance;
            if(current_distance == shortest_distance[starting_point]){
                start = nodes[ending_point];
                end = nodes[starting_point];
              //  std::cout<<start<<" "<<end<< std::endl;
                path = adj_matrix[starting_point][ending_point].subpath;


                std::cout<<path[0]<<" ";
                std::cout<<path[path.size()-1]<<" ";
                std::cout<<"banana\n";
                pickUp_indices.push_back((starting_point/2));
                final_path.push_back({start, end, path, pickUp_indices});
                visited[ending_point] = true;
                //weight += deliveries[ending_point/2].itemWeight;
               if(starting_point ==0){
                    starting_point = ending_point;
                    ending_point = 1;
                }else{
                    starting_point = ending_point;
                    ending_point = 0;
                }
                start = end;
            }
        }
        else if(ending_point%2 != 0 && !visited[ending_point] && visited[ending_point-1]){
                start = nodes[ending_point - 1];
                end = nodes[ending_point];
                path = adj_matrix[starting_point][ending_point].subpath;
                std::cout<<path[0]<<" ";
                        std::cout<<path[path.size()-1]<<" ";

                        std::cout<<"banana\n";
                final_path.push_back({start, end, path, drop_offs});
                visited[ending_point] = true;
                //weight -= deliveries[ending_point/2].itemWeight;
                final_index = ending_point;
                starting_point = ending_point;
                ending_point = 0;
        }
       
    }  
    
    for(int i =0; i<visited.size(); i++){
        std::cout<<visited[i]<<" ";
    }
    
    start = nodes[final_index];
    end = find_nearest_depot(nodes[final_index], depots);
    path = find_path_between_intersections(start, end, right_turn_penalty, left_turn_penalty);
    final_path.push_back({start, end, path, pickUp_indices});
    
    return final_path;
}

std::vector<unsigned> graph_deliveries(std::vector<DeliveryInfo> deliveries){
    std::vector<unsigned> nodes;
    int j=0;
    int k=0;
    for(int i = 0; i< (2*deliveries.size()); i++){     
        if(i%2 == 0){      
            nodes.push_back(deliveries[j].pickUp);
            j++;
        }
        else{
            nodes.push_back(deliveries[k].dropOff);
            k++;
        }
    }
    
    return nodes;
}

//function that finds the nearest depot (by intersection id) to a certain intersection based on geometric distance
unsigned find_nearest_depot(unsigned intersection_id, const std::vector<unsigned>& depots){
    LatLon start = getIntersectionPosition(intersection_id);
    LatLon end = getIntersectionPosition(depots[0]);
    unsigned depot_id = depots[0];
    //finding the initial distance
    double shortest_distance = find_distance_between_two_points(start, end);
    
    //looping through all the depots
    for(unsigned i = 1; i < depots.size(); i++){
        LatLon temp_end = getIntersectionPosition(depots[i]);
        double temp_shortest_distance = find_distance_between_two_points(start, temp_end);
        //if there is a closer depot found, update the distance and id
        if(temp_shortest_distance < shortest_distance){
            shortest_distance = temp_shortest_distance;
            depot_id = depots[i];
        }
    }
    
    return depot_id;
}
//function that finds the nearest depot (by intersection id) to a certain intersection based on geometric distance
/*DeliveryInfo find_nearest_pickup(unsigned intersection_id, const std::vector<DeliveryInfo>& deliveries){
    LatLon start = getIntersectionPosition(intersection_id);
    std::vector<unsigned> pickUps;   
 
    
    for(unsigned i = 0; i < deliveries.size(); i++){
        pickUps.push_back(deliveries[i].pickUp);
    }

    LatLon end = getIntersectionPosition(pickUps[0]);
    unsigned pickUp_id = pickUps[0];
    //finding the initial distance
    double shortest_distance = find_distance_between_two_points(start, end);
    
    //looping through all the depots
    for(unsigned j = 1; j < pickUps.size(); j++){
        LatLon temp_end = getIntersectionPosition(pickUps[j]);
        double temp_shortest_distance = find_distance_between_two_points(start, temp_end);
        //if there is a closer depot found, update the distance and id
        if(temp_shortest_distance < shortest_distance){
            shortest_distance = temp_shortest_distance;
            pickUp_id = pickUps[j];
        }
    }
    
    return pickUp_id;
}

/*std::vector<unsigned> pickups_duplicate;
std::vector<unsigned> dropoffs_duplicate;
DeliveryInfo find_nearest_pickup(unsigned intersection_id, const std::vector<DeliveryInfo>& deliveries);
DeliveryInfo find_nearest_dropoff(unsigned intersection_id, const std::vector<DeliveryInfo>& deliveries);
bool check_pickup_legality(unsigned pickup_id);
bool check_dropoff_legality(unsigned dropoff_id);
unsigned find_nearest_depot(unsigned intersection_id, const std::vector<unsigned>& depots);
//bool DropOff(DeliveryInfo currentDelivery, DeliveryInfo nextDelivery);

std::vector<CourierSubpath> traveling_courier(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots, const float right_turn_penalty, const float left_turn_penalty, const float truck_capacity){
    int N = deliveries.size();
    float weight = 0;
    std::vector<CourierSubpath> final_path;
    std::vector<CourierSubpath> empty_path;
    
    for(int i = 0; i< N-1; i++){
        pickups_duplicate.push_back(deliveries[i].pickUp); 
    }
    
    // Go from first depot to first package pick up
    unsigned first_start = depots[0];
    unsigned first_end = find_nearest_pickup(depots[0], deliveries).pickUp;
    std::vector<unsigned> first_path = find_path_between_intersections(first_start, first_end, right_turn_penalty, left_turn_penalty);
    
    if(first_path.size() == 0)
        return empty_path;
    
    std::vector<unsigned> first_pickUp;
    final_path.push_back({first_start, first_end, first_path, first_pickUp});
    DeliveryInfo current_delivery = find_nearest_pickup(depots[0], deliveries);
    check_pickup_legality(current_delivery.pickUp);
    weight = current_delivery.itemWeight;
    
    DeliveryInfo next_delivery = DeliveryInfo(0, 0, 0);
    DeliveryInfo next_dropOff = DeliveryInfo(0, 0, 0);
    unsigned temp_start;
    unsigned temp_end;
    std::vector<unsigned> temp_path;
    unsigned temp_pickUp_index;
    std::vector<unsigned> temp_pickup_indices;
    int i = 1;
    
    
     // Complete deliveries, in order
    while(pickups_duplicate.size() > 0){
        
        next_delivery = find_nearest_pickup(current_delivery.pickUp, deliveries);
        if((weight + next_delivery.itemWeight) < truck_capacity){
            if(check_pickup_legality(next_delivery.pickUp)){
                temp_start = first_end;
                temp_end = next_delivery.pickUp;
                temp_path = find_path_between_intersections(temp_start, temp_end, right_turn_penalty, left_turn_penalty);
                
                if(temp_path.size() == 0)
                    return empty_path;
                
                temp_pickUp_index = i;
                temp_pickup_indices.push_back(temp_pickUp_index);
                final_path.push_back({temp_start, temp_end, temp_path, temp_pickup_indices});

                current_delivery = next_delivery;
                weight += current_delivery.itemWeight;
                first_end = temp_end;
                dropoffs_duplicate.push_back(current_delivery.dropOff);
            }
        }else{
            temp_start = first_end;
            temp_end = current_delivery.dropOff;
            
            if(check_dropoff_legality(temp_end)){
                temp_path = find_path_between_intersections(temp_start, temp_end, right_turn_penalty, left_turn_penalty);
                
                if(temp_path.size() == 0)
                    return empty_path;
                
                final_path.push_back({temp_start, temp_end, temp_path,});

                weight -= current_delivery.itemWeight;
                first_end = temp_end;    
            }           
        }
        
        i++;
    }
    
    while(dropoffs_duplicate.size() > 0){
        next_dropOff = find_nearest_dropoff(current_delivery.dropOff, deliveries); 
        temp_start = first_end;
        temp_end = next_dropOff.dropOff;
        
        if(check_dropoff_legality(temp_end)){
            temp_path = find_path_between_intersections(temp_start, temp_end, right_turn_penalty, left_turn_penalty);
            
            if(temp_path.size() == 0)
                return empty_path;
            
            final_path.push_back({temp_start, temp_end, temp_path,});

            weight -= next_dropOff.itemWeight;
            current_delivery = next_dropOff;
            first_end = temp_end;
        }
    }
 
    // Go back to the first depot to drop off the truck
    unsigned last_start = deliveries[N - 1].dropOff;
    unsigned last_end = find_nearest_depot(last_start, depots);
    std::vector<unsigned> last_path = find_path_between_intersections(last_start, last_end, right_turn_penalty, left_turn_penalty);
    
    if(last_path.size() == 0)
        return empty_path;
    
    std::vector<unsigned> no_pickUp;
    final_path.push_back({last_start, last_end, last_path, no_pickUp});
    
    for(int j =0; j< final_path.size(); j++){
        std::cout<<final_path[i];
    }
    return final_path;
}

bool check_pickup_legality(unsigned pickup_id){ 
    for(unsigned i= 0; i<pickups_duplicate.size(); i++){
        if(pickups_duplicate[i] == pickup_id){
            pickups_duplicate.erase(pickups_duplicate.begin() + i);
            return true;
        }
    }
    return false;
}

bool check_dropoff_legality(unsigned dropoff_id){
    for(unsigned i= 0; i<dropoffs_duplicate.size(); i++){
        if(dropoffs_duplicate[i] == dropoff_id){
            dropoffs_duplicate.erase(dropoffs_duplicate.begin() + i);
            return true;;
        }
    }
    return false;
}

//function that finds the nearest depot (by intersection id) to a certain intersection based on geometric distance
DeliveryInfo find_nearest_pickup(unsigned intersection_id, const std::vector<DeliveryInfo>& deliveries){
    LatLon start = getIntersectionPosition(intersection_id);
//    std::vector<unsigned> pickUps;
//    
//    for(unsigned i = 0; i < deliveries.size(); i++){
//        pickUps.push_back(deliveries[i].pickUp);
//    }

    LatLon end = getIntersectionPosition(pickups_duplicate[0]);
    unsigned pickUp_id = pickups_duplicate[0];
    //finding the initial distance
    double shortest_distance = find_distance_between_two_points(start, end);
    
    unsigned j;
    //looping through all the depots
    for(j = 1; j < pickups_duplicate.size(); j++){
        LatLon temp_end = getIntersectionPosition(pickups_duplicate[j]);
        double temp_shortest_distance = find_distance_between_two_points(start, temp_end);
        //if there is a closer depot found, update the distance and id
        if(temp_shortest_distance < shortest_distance){
            shortest_distance = temp_shortest_distance;
            pickUp_id = pickups_duplicate[j];
        }
    }
    
    //finding the delivery info for the closest pickup
    DeliveryInfo next_delivery = DeliveryInfo(0, 0, 0);
    for(unsigned i = 0; i < deliveries.size(); i++){
        if(deliveries[i].pickUp == pickUp_id){
            next_delivery = deliveries[i];
        }
    }
    
    return next_delivery;
}

//function that finds the nearest depot (by intersection id) to a certain intersection based on geometric distance
DeliveryInfo find_nearest_dropoff(unsigned intersection_id, const std::vector<DeliveryInfo>& deliveries){
    LatLon start = getIntersectionPosition(intersection_id);
//    std::vector<unsigned> pickUps;
//    
//    for(unsigned i = 0; i < deliveries.size(); i++){
//        pickUps.push_back(deliveries[i].pickUp);
//    }

    LatLon end = getIntersectionPosition(dropoffs_duplicate[0]);
    unsigned dropOff_id = dropoffs_duplicate[0];
    //finding the initial distance
    double shortest_distance = find_distance_between_two_points(start, end);
    
    unsigned j;
    //looping through all the depots
    for(j = 1; j < dropoffs_duplicate.size(); j++){
        LatLon temp_end = getIntersectionPosition(dropoffs_duplicate[j]);
        double temp_shortest_distance = find_distance_between_two_points(start, temp_end);
        //if there is a closer depot found, update the distance and id
        if(temp_shortest_distance < shortest_distance){
            shortest_distance = temp_shortest_distance;
            dropOff_id = dropoffs_duplicate[j];
        }
    }
    
    //finding the delivery info for the closest pickup
    DeliveryInfo next_delivery = DeliveryInfo(0, 0, 0);
    for(unsigned i = 0; i < deliveries.size(); i++){
        if(deliveries[i].dropOff == dropOff_id){
            next_delivery = deliveries[i];
        }
    }
    
    return next_delivery;
}

//function that finds the nearest depot (by intersection id) to a certain intersection based on geometric distance
unsigned find_nearest_depot(unsigned intersection_id, const std::vector<unsigned>& depots){
    LatLon start = getIntersectionPosition(intersection_id);
    LatLon end = getIntersectionPosition(depots[0]);
    unsigned depot_id = depots[0];
    //finding the initial distance
    double shortest_distance = find_distance_between_two_points(start, end);
    
    //looping through all the depots
    for(unsigned i = 1; i < depots.size(); i++){
        LatLon temp_end = getIntersectionPosition(depots[i]);
        double temp_shortest_distance = find_distance_between_two_points(start, temp_end);
        //if there is a closer depot found, update the distance and id
        if(temp_shortest_distance < shortest_distance){
            shortest_distance = temp_shortest_distance;
            depot_id = depots[i];
        }
    }
    
    return depot_id;
}

bool DropOff(DeliveryInfo currentDelivery, DeliveryInfo nextDelivery){
   double drop_off_distance;
   double pick_up_distance;
   
   LatLon current_pickup = getIntersectionPosition(currentDelivery.pickUp);
   LatLon current_dropoff = getIntersectionPosition(currentDelivery.dropOff);
   LatLon next_pickup = getIntersectionPosition(nextDelivery.pickUp);
   
   drop_off_distance = find_distance_between_two_points(current_pickup, current_dropoff);
   pick_up_distance = find_distance_between_two_points(current_pickup, next_pickup);
   
   if(pick_up_distance >= drop_off_distance)
       return true;
   else
       return false;
}*/

