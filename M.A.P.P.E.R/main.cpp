/* 
 * Copyright 2019 University of Toronto
 *
 * 
 *ECE297 Winter 2019
 * group cd-092
 * 
 * Authors:
 * Abbas Majed
 * Omar
 * Neehar
 * Scott
 * 
 * February 1 2019
 * Time last entered : 11:37 am
 */
#include <iostream>
#include <string>
#include "m1.h"
#include "m2.h"
#include "m3.h"

int enterCityName();

//Program exit codes
constexpr int SUCCESS_EXIT_CODE = 0;        //Everyting went OK
constexpr int ERROR_EXIT_CODE = 1;          //An error occured
constexpr int BAD_ARGUMENTS_EXIT_CODE = 2;  //Invalid command-line usage

//The default map to load if none is specified
std::string default_map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";


// create a vector cityMap that has all 19 city names in order
std::vector<std::string> cityMap = {"toronto_canada","cairo_egypt","beijing_china", "tehran_iran","iceland","rio-de-janeiro_brazil","golden-horseshoe_canada","new-delhi_india","new-york_usa","london_england","moscow_russia","tokyo_japan","interlaken_switzerland","hong-kong_china","sydney_australia","cape-town_south-africa","hamilton_canada","saint-helena","singapore"};         

// declare the global map city number used
int newMapCityNumber;

// function enterCityName used to take in input user city name 
int enterCityName(){
    
    //get user input from these only existing 19 cities names
    
    std::cout << "Enter City Number: \n";
    std::cout << "0. Exit Map: \n";
    std::cout << "1. Toronto \n";
    std::cout << "2. Cairo \n";
    std::cout << "3. Beijing \n";
    std::cout << "4. Tehran \n";
    std::cout << "5. Iceland \n";
    std::cout << "6. Rio De Janeiro \n";
    std::cout << "7. Golden Horseshoe \n";
    std::cout << "8. New Delhi \n";
    std::cout << "9. New York \n";
    std::cout << "10. London\n";
    std::cout << "11. Moscow \n";
    std::cout << "12. Tokyo \n";
    std::cout << "13. Interlaken \n";
    std::cout << "14. Hong Kong \n";
    std::cout << "15. Sydney \n";
    std::cout << "16. Cape Town \n";
    std::cout << "17. Hamilton \n";
    std::cout << "18. Saint-Helena \n";
    std::cout << "19. Singapore \n";
    std::cout << "----------------------\n";
    std::cout << "City Number: "<<std::endl;
     
   // to avoid misspelled, case sensitivity, partial names and incorrect format
   // user will choose designated city number matching their chosen city
    
    int cityNumber;
    std::cin >> cityNumber;        // newMapNumber 
    
    return cityNumber;
    
}

int main(int argc, char** argv) {
    
    // while loop used to iterate different maps before closing
    // this will prevent recompilation 
    // you can choose map user wants until exiting map with return code
    
    while (1==1){ 
        
        
        newMapCityNumber= enterCityName();       // call this function to take user chosen city number
        
        // check for the validity of the user input 
        
        if(newMapCityNumber >= 0 && newMapCityNumber <= 19){

            if (newMapCityNumber == 0 ){
                return SUCCESS_EXIT_CODE;
            }
        
            std:: string map_path;
            if(argc == 1) {
            //Use a default map
                map_path = "/cad2/ece297s/public/maps/"+cityMap[newMapCityNumber-1]+ ".streets.bin";
            }
            else if (argc == 2) {
                //Get the map from the command line 
                map_path = argv[1];
            }
            else {
            //Invalid arguments
                std::cerr << "Usage: " << argv[0] << " [map_file_path]\n";
                std::cerr << "  If no map_file_path is provided a default map is loaded.\n";
                return BAD_ARGUMENTS_EXIT_CODE;
            }
    
            //Load the map and related data structures
            bool load_success = load_map(map_path);
            if(!load_success) {
                std::cerr << "Failed to load map '" << map_path << "'\n";
                return ERROR_EXIT_CODE;
            }

            std::cout << "Successfully loaded map '" << map_path << "'\n";
        
            std::cout << "Drawing map\n";
            draw_map();
    
            //Clean-up the map data and related data structures
            std::cout << "Closing map\n";
            close_map(); 
            }
        
            std::cout << "Invalid input, please enter another number. \n";
            //return SUCCESS_EXIT_CODE;
    }
    
}    
