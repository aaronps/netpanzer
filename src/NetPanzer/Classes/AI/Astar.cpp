/*
Copyright (C) 1998 Pyrosoft Inc. (www.pyrosoftgames.com), Matthew Bogue

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <config.h>
#include "Astar.hpp"

#include "Timer.hpp"
#include "PathingState.hpp"

int compare( PObject *object1, PObject *object2 )
 {
  AstarNode *node1, *node2;
  node1 = (AstarNode *) object1; 					   
  node2 = (AstarNode *) object2;
 
  if (node1->f > node2->f)
   return( 1 );
  else
   if ( node1->f < node2->f )
    return( -1 );
    
  return( 0 ); 
 } 


Astar::Astar( void )
 {
  node_list = 0;
 }

void Astar::initializeAstar( unsigned long node_list_size,
                             unsigned long step_limit,
			       		     long heuristic_weight )
 { 
  open_set.initialize( getMapXsize(),  getMapYsize() );
  closed_set.initialize( getMapXsize(), getMapYsize() );
  
  open.initialize( node_list_size + (100) , node_list_size / 10, 0xFFFFFFFF );
  open.setCompare( compare );

  initializeNodeList( node_list_size );

  Astar::step_limit = step_limit;
  Astar::heuristic_weight = heuristic_weight;
 
  ini_flag = true; 
 }

void Astar::initializeAstar( unsigned long node_list_size )
 {
  initializeAstar( node_list_size, 50, 30 );
 }


void Astar::initializeAstar( void )
 {
  initializeAstar( 4000, 50, 30 );
 }

 
AstarNode *  Astar::getNewNode( void )
 {
  AstarNode *node_ptr;
  
 if ( dynamic_node_management_flag == true )
  {
   if (free_list_ptr == 0) 
  	{
	 return( 0 );
	}
   else
    {
	 node_ptr = free_list_ptr;
	 free_list_ptr = free_list_ptr->parent;
	 return( node_ptr );
	}
  
  } 
 else
  { 
   if ( node_index >= node_list_size )
    {
     return( 0 );
    }

   node_ptr = &node_list[ node_index ];
   node_index++;
    
   return( node_ptr );  
  }
  
 }

void Astar::releaseNode( AstarNode *node )
 {
  if ( dynamic_node_management_flag == true )
   {
	node->parent = free_list_ptr;
	free_list_ptr = node;
   }
 }


void Astar::resetNodeList( void )
 {
  node_index = 0;
  
  if ( dynamic_node_management_flag == true )
   {
	int node_list_index;
	int node_list_start;

	node_list_start = node_list_size - 2;
	
	node_list[ node_list_size - 1].parent = 0;

	for ( node_list_index = node_list_start; node_list_index >= 0; node_list_index-- )
	 {
	  node_list[ node_list_index ].parent = &(node_list[ node_list_index + 1]);
	 }

   	free_list_ptr = &(node_list[ 0 ]);
   }

 }

void Astar::initializeNodeList( unsigned long initial_size )
 {
  node_index = 0;
  node_list_size = initial_size;

  if ( node_list != 0 )
   {
    free( node_list );
    node_list = 0;
   }
  
  node_list = (AstarNode *) malloc( sizeof( AstarNode) * node_list_size );
  assert( node_list != 0 );
 
 }
  

void Astar::initializePath( PointXYi &start, PointXYi &goal, unsigned short path_type )
 {
  TIMESTAMP timer_ini_mark = now();

  if ( sample_set_array_flag == true )
   start_sampling_flag = true;
    
  resetNodeList();
  
  total_pathing_time = 0;
  steps = 0;
  total_steps = 0;
  succ_swap_flag = false;
  path_type_flag = path_type;

  goal_node.map_loc = goal;
 
  goal_node.g = getMovementValue( goal );
  goal_node.abs_loc = mapXYtoAbsloc( goal );
  
  best_node = 0;
  best_node = getNewNode();

  assert( best_node != 0 );

  best_node->map_loc = start;
  best_node->abs_loc = mapXYtoAbsloc( start );
  best_node->g = 0;
  best_node->h = heuristic( best_node->map_loc, goal );
  best_node->f = best_node->g + best_node->h;
  best_node->parent = 0;
  
  open.push( best_node );
  
  open_set.setBit( best_node->map_loc.x, best_node->map_loc.y );  
 
  total_pathing_time += now() - timer_ini_mark;
 }                  

unsigned long Astar::mapXYtoAbsloc( PointXYi map_loc )
 {
  unsigned long abs;

  if ( ( map_loc.x < 0 ) || (map_loc.x >= main_map.getXsize() ) || 
       ( map_loc.y < 0 ) || (map_loc.y >= main_map.getYsize() )
     )
  return( 0xFFFFFFFF );

  main_map.mapXYtoOffset( map_loc.x, map_loc.y, &abs );
  
  return( abs );  
 }


long Astar::heuristic( PointXYi &pointA, PointXYi &pointB )
 {
  long delta_x;
  long delta_y;
  
  delta_x = labs( pointA.x - pointB.x );
  delta_y = labs( pointA.y - pointB.y );

  return( heuristic_weight * (delta_x + delta_y) );
 }


unsigned char Astar::generateSucc( unsigned short direction, AstarNode *node, AstarNode *succ ) 
 {
  unsigned long movement_val;

  if (succ_swap_flag)
   {
    direction = 7 - direction; 
   }

  switch( direction )
   {
    case 0: { succ->map_loc.x =  1; succ->map_loc.y =  0; succ->g = 1; } break;
    case 1: { succ->map_loc.x =  1; succ->map_loc.y = -1; succ->g = 0; } break;
    case 2: { succ->map_loc.x =  0; succ->map_loc.y = -1; succ->g = 1; } break;
    case 3: { succ->map_loc.x = -1; succ->map_loc.y = -1; succ->g = 0; } break;
    case 4: { succ->map_loc.x = -1; succ->map_loc.y =  0; succ->g = 1; } break;
    case 5: { succ->map_loc.x = -1; succ->map_loc.y =  1; succ->g = 0; } break;
    case 6: { succ->map_loc.x =  0; succ->map_loc.y =  1; succ->g = 1; } break;
    case 7: { succ->map_loc.x =  1; succ->map_loc.y =  1; succ->g = 0; } break;
   }
 
  succ->map_loc = node->map_loc + succ->map_loc;
  succ->abs_loc = mapXYtoAbsloc( succ->map_loc );

  movement_val = getMovementValue( succ->map_loc );

  if ( movement_val != 0xFF )
   if ( ( (UnitBlackBoard::unitOccupiesLoc( succ->map_loc ) == true ) &&
          (succ->map_loc != goal_node.map_loc) )                 
      )
    {
     movement_val = 200;
    }
   
  succ->g += node->g + movement_val;
   
  succ->h = heuristic( succ->map_loc, goal_node.map_loc );
  succ->f = succ->h + succ->g ;

  succ->parent = 0;

  return( movement_val );
 }

bool Astar::generatePath( PathRequest *path_request, 
                             unsigned short path_merge_type, 
                             bool dynamic_node_managment,
                             int *result_code )
{
  if ( ini_flag )
   { 
    Astar::path_request_ptr = path_request;
	Astar::path_merge_type = path_merge_type;
	Astar::dynamic_node_management_flag = dynamic_node_managment;
	initializePath( path_request->start, path_request->goal, path_request->path_type ); 
    ini_flag = false; 
   }

  return ( process_succ( path_request->path, result_code ) );
}

bool Astar::process_succ( PathList *path, int *result_code )
 {
  AstarNode *node;
  AstarNode temp_node;
  unsigned long temp;
  bool done = false;
  unsigned short succ_loop;
  bool steps_comp = false ;
  bool goal_reachable = true;
  unsigned short path_length;

  unsigned short *map_buffer;
 
  main_map.getRawMapBuffer( &map_buffer );

  TIMESTAMP timer_path_mark = now();
  
  while( !done && !steps_comp )
   {   
	best_node = (AstarNode *) open.pop( );
    
    if ( best_node == 0 )
    {
     goal_reachable = false;
     done = true;
    }
    else
    { 
    if ( (best_node->map_loc == goal_node.map_loc) )
     {
      done = true;
     }
    else
     { 
       for ( succ_loop = 0; succ_loop < 8; succ_loop++ )
        {
         unsigned char movement_value;
         movement_value = generateSucc( succ_loop, best_node, &temp_node );
         
         if ( movement_value != 0xFF )
          {
           if ( open_set.getBit( temp_node.map_loc.x, temp_node.map_loc.y ) )
            {
             //best_node->child[ succ_loop ] = old;
         
            } // ** if in open
           else
            if ( closed_set.getBit( temp_node.map_loc.x, temp_node.map_loc.y ) )
             {
              //best_node->child[ succ_loop ] = old;
          
             } // ** if in closed 
            else
             {
              node = 0; 
              
              node = getNewNode();
              
              if ( node == 0 )
               {
                done = true;
                goal_reachable = false; 
               }
               else
                {
                 memcpy( node, &temp_node, sizeof( AstarNode ) ); 
                 node->parent = best_node;
                

                 open_set.setBit( node->map_loc.x, node->map_loc.y );
                 
				 if ( start_sampling_flag == true)
                  astar_set_array.setBit( node->map_loc.x, node->map_loc.y );
				  
                 open.push( node );
              
                 }
             
             } // ** else
        
          } // ** if valid succesor
        }  // ** for succ_loop
     
     } // **else

     open_set.clearBit( best_node->map_loc.x, best_node->map_loc.y );
 
	 closed_set.setBit( best_node->map_loc.x, best_node->map_loc.y );	

 	 if ( start_sampling_flag == true)
      astar_set_array.setBit( best_node->map_loc.x, best_node->map_loc.y );	

  	 releaseNode( best_node );
 
    } // ** else
	
	if ( succ_swap_flag )
     succ_swap_flag = false;
    else
     succ_swap_flag = true;

    if ( steps > step_limit )
     {
      steps_comp = true;
      steps = 0;
     }
    else
     {
      steps++;
      total_steps++;
     }
  
  } // ** while
 
  total_pathing_time += now() - timer_path_mark;

   if ( done && goal_reachable )
    {
     bool insert_successful = true;
	 path_length = 0;
     
	 node = best_node;
     while ( (node != 0) && (insert_successful == true) )
      {
       
       if ( path_merge_type == _path_merge_front )
	    { insert_successful = path->pushFirst( node->abs_loc ); }
	   else
		{ insert_successful = path->pushLast( node->abs_loc ); }
	   	   
	   node = node->parent; 
       path_length++;
       
	  }
	 
	 if ( insert_successful == false ) 
	  {
	   path->reset();
	  }
     else
	  { 
	   if ( path_merge_type == _path_merge_front )
	    { path->popFirst( &temp ); }
	   else
	   { path->popLast( &temp ); }
      }

     cleanUp();
     ini_flag = true;
	 
	 PathingState::path_length = path_length;
	 PathingState::astar_gen_time = total_pathing_time;
     PathingState::astar_gen_time_total += total_pathing_time;
	 *result_code = _path_result_success;
     return( true );
    }
   else
    if ( !goal_reachable )
     {
      ini_flag = true;
      cleanUp(); 
      
	  PathingState::astar_gen_time = total_pathing_time;
	  PathingState::astar_gen_time_total += total_pathing_time;
	  *result_code = _path_result_goal_unreachable;
      return( true );
     }

   return( false );
 }

void Astar::cleanUp( void )
 {
  TIMESTAMP timer_cleanup_mark = now();

  open_set.clear();
  closed_set.clear(); 
  open.reset();

  resetNodeList();
  ini_flag = true;
 
  sample_set_array_flag = false;
  start_sampling_flag = false;	 
  
  total_pathing_time += now() - timer_cleanup_mark;
 }

void Astar::setDebugMode( bool on_off )
 {
  debug_mode_flag = on_off;

  if ( debug_mode_flag == true )
   {
	astar_set_array.initialize( MapInterface::getMapXsize(), MapInterface::getMapYsize() );
   }
  else
   {
	astar_set_array.deallocate();
   }
 }

void Astar::sampleSetArrays( void )
 {
  if ( debug_mode_flag == true )
   {
	astar_set_array.clear();
    sample_set_array_flag = true;
    start_sampling_flag = false;	 
   }
 }
 
BitArray * Astar::getSampledSetArrays( void )
 {
  return( &astar_set_array );
 }
