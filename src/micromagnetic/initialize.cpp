//------------------------------------------------------------------------------
//
//   This file is part of the VAMPIRE open source package under the
//   Free BSD licence (see licence file for details).
//
//   (c) Sarah Jenkins and Richard F L Evans 2016. All rights reserved.
//
//   Email: sj681@york.ac.uk
//
//------------------------------------------------------------------------------
//

// C++ standard library headers

// Vampire headers
#include "micromagnetic.hpp"

// micromagnetic module headers
#include "internal.hpp"
#include "material.hpp"
#include "cells.hpp"
#include "atoms.hpp"


#include <iostream>

namespace mm = micromagnetic::internal;
using namespace std;

namespace micromagnetic{

   //----------------------------------------------------------------------------
   // Function to initialize micromagnetic module
   //----------------------------------------------------------------------------

   void initialize(int num_local_cells,
                   int num_cells,
                   int num_atoms,
                   int num_materials,
                   std::vector<int> cell_array,                     //1D array storing which cell each atom is in
                   std::vector<int> neighbour_list_array,           //1D vector listing the nearest neighbours of each atom
                   std::vector<int> neighbour_list_start_index,     //1D vector storing the start index for each atom in the neighbour_list_array
                   std::vector<int> neighbour_list_end_index,       //1D vector storing the end index for each atom in the neighbour_list_array
                   const std::vector<int> type_array,               //1D array storing which material each cell is
                   std::vector <mp::materials_t> material,          //1D vector of type material_t stiring the material properties
                   std::vector <double> x_coord_array,
                   std::vector <double> y_coord_array,
                   std::vector <double> z_coord_array,
                   std::vector <double> volume_array,               //1D vector storing the volume of each cell
                   double Temperature,
                   double num_atoms_in_unit_cell,
                   double system_dimensions_x,
                   double system_dimensions_y,
                   double system_dimensions_z,
                   std::vector<int> local_cell_array){


   //resizes the vectors used to store the cell parameters
   mm::A.resize(num_cells*num_cells,0.0);
   mm::alpha.resize(num_cells,0.0);
   mm::one_o_chi_perp.resize(num_cells,0.0);
   mm::one_o_chi_para.resize(num_cells,0.0);
   mm::gamma.resize(num_cells,0.0);
   mm::ku.resize(num_cells,0.0);
   mm::ms.resize(num_cells,0.0);
   mm::Tc.resize(num_cells,0.0);

   mm::alpha_para.resize(num_cells,0.0);
   mm::alpha_perp.resize(num_cells,0.0);

   mm::m_e.resize(num_cells,0.0);

   mm::macro_neighbour_list_start_index.resize(num_cells,0.0);
   mm::macro_neighbour_list_end_index.resize(num_cells,0.0);

   micromagnetic::cell_discretisation_micromagnetic.resize(num_cells,true);

   mm::ext_field.resize(3,0.0);



   //These functions vectors with the parameters calcualted from the function
   mm::ms =          mm::calculate_ms(num_local_cells,num_atoms,num_cells, cell_array, type_array,material,local_cell_array);
   mm::alpha =       mm::calculate_alpha(num_local_cells,num_atoms, num_cells, cell_array, type_array, material,local_cell_array);
   mm::Tc =          mm::calculate_tc(num_local_cells, local_cell_array,num_atoms, num_cells, cell_array,neighbour_list_array,
                                       neighbour_list_start_index, neighbour_list_end_index, type_array, material);
   mm::ku =          mm::calculate_ku(num_atoms, num_cells, cell_array, type_array, material);
   mm::gamma =       mm::calculate_gamma(num_atoms, num_cells, cell_array,type_array,material);
   mm::one_o_chi_para =    mm::calculate_chi_para(num_local_cells, local_cell_array,num_cells, Temperature);
   mm::one_o_chi_perp =    mm::calculate_chi_perp(num_local_cells, local_cell_array,num_cells, Temperature);
   mm::A =           mm::calculate_a(num_atoms, num_cells, num_local_cells,cell_array, neighbour_list_array, neighbour_list_start_index,
                                    neighbour_list_end_index, type_array,  material, volume_array, x_coord_array,
                                    y_coord_array, z_coord_array, num_atoms_in_unit_cell, local_cell_array);

//for (int lc = 0; lc < num_local_cells; lc++)
//{
 // int cell = local_cell_array[lc];
//std::cout << cell << '\t' <<mm::ms[cell] << '\t' << mm::alpha[cell]<< '\t' << mm::gamma[cell] <<'\t' << mm::A[cell] << "\t" << mm::chi_para[cell] << '\t' << mm::chi_perp[cell] << "\t" << mm::Tc[cell] << "\t" << mm::ku[cell] << std::endl;
//}



//if multiscale simulation
if (discretisation_type == 2){

   std::cout<< "multiscale" <<std::endl;
   for (int atom =0; atom < num_atoms; atom++){
      int cell = cell_array[atom];
      int mat  = type_array[atom];
      micromagnetic::cell_discretisation_micromagnetic[cell] = mp::material[mat].micromagnetic_enabled;
      if (mm::Tc[cell] < 0) micromagnetic::cell_discretisation_micromagnetic[cell] = 0;
   }

   for (int lc = 0; lc < num_local_cells; lc++){
      int cell = local_cell_array[lc];
   }
   for (int atom =0; atom < num_atoms; atom++){
      int cell = cell_array[atom];
      int mat  = type_array[atom];
  //if (micromagnetic::cell_discretisation_micromagnetic[cell] ==0 && mm::ms[cell] )  std::cout << "cells" << '\t' << atom << '\t' << cell <<'\t' <<micromagnetic::cell_discretisation_micromagnetic[cell] <<std::endl;
      if (micromagnetic::cell_discretisation_micromagnetic[cell] == 0) {
         list_of_atomistic_atoms.push_back(atom);
         number_of_atomistic_atoms++;
      }
      else {
        list_of_none_atomistic_atoms.push_back(atom);
        number_of_none_atomistic_atoms++;
      }
   }
   std::cout <<"atoms" << number_of_atomistic_atoms << '\t' << atoms::num_atoms <<std::endl;

  for (int lc = 0; lc < num_local_cells; lc++){
      int cell = local_cell_array[lc];
    if (micromagnetic::cell_discretisation_micromagnetic[cell] == 1 && mm::ms[cell] > 1e-30) {
      list_of_micromagnetic_cells.push_back(cell);
      number_of_micromagnetic_cells ++;
    }
  }

}
else {
   for (int lc = 0; lc < num_local_cells; lc++){
       int cell = local_cell_array[lc];
    list_of_micromagnetic_cells.push_back(cell);
    number_of_micromagnetic_cells ++;
    }
    for (int atom =0; atom < num_atoms; atom++){
       int cell = cell_array[atom];
       int mat  = type_array[atom];
      list_of_none_atomistic_atoms.push_back(atom);
      number_of_none_atomistic_atoms++;
    }

  }

     std::cout << "cells" << number_of_micromagnetic_cells<< '\t' << number_of_atomistic_atoms <<std::endl;
     if (number_of_atomistic_atoms > 0){
     int end = list_of_atomistic_atoms[0];
     int begin = list_of_atomistic_atoms[0];
     for(int atom_list=1;atom_list<number_of_atomistic_atoms;atom_list++){
       int atom = list_of_atomistic_atoms[atom_list];
       int last_atom = list_of_atomistic_atoms[atom_list - 1];
       if ((atom != last_atom +1) || (atom_list == number_of_atomistic_atoms -1)){
         end = atom +1;
         mm::fields_neighbouring_atoms_begin.push_back(begin);
         mm::fields_neighbouring_atoms_end.push_back(end);

         begin = atom + 1;
       }
    }}

     int num_calculations = mm::fields_neighbouring_atoms_begin.size();
     std::cout << num_calculations <<std::endl;





      return;

   }

} // end of micromagnetic namespace
