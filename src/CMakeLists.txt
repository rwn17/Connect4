CMAKE_MINIMUM_REQUIRED(VERSION 3.5)
PROJECT(AI_project) 
AUX_SOURCE_DIRECTORY(. SRC_LIST) 

add_library(My_strategy SHARED ${SRC_LIST})
 
set_target_properties(My_strategy PROPERTIES output_name "My_strategy")