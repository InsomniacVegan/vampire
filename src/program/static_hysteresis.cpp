//-----------------------------------------------------------------------------
//
//  Vampire - A code for atomistic simulation of magnetic materials
//
//  Copyright (C) 2009-2012 R.F.L.Evans
//
//  Email:richard.evans@york.ac.uk
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// ----------------------------------------------------------------------------
//
///
/// @file
/// @brief Contains the Static Hysteresis program
///
/// @details Performs a field loop to determine field dependent magnetic behaviour
///
/// @section License
/// Use of this code, either in source or compiled form, is subject to license from the authors.
/// Copyright \htmlonly &copy \endhtmlonly Richard Evans, 2009-2010. All Rights Reserved.
///
/// @section info File Information
/// @author  Richard Evans, richard.evans@york.ac.uk
/// @version 1.1
/// @date    10/03/2011
/// @internal
///	Created:		05/02/2011
///	Revision:	10/03/2011
///=====================================================================================
///

// Standard Libraries
#include <cstdlib>

// Vampire Header files
#include "vmath.hpp"
#include "errors.hpp"
#include "sim.hpp"
#include "stats.hpp"
#include "vio.hpp"

namespace program{

/// @brief Function to calculate a static hysteresis loop
///
/// @section License
/// Use of this code, either in source or compiled form, is subject to license from the authors.
/// Copyright \htmlonly &copy \endhtmlonly Richard Evans, 2009-2010. All Rights Reserved.
///
/// @section Information
/// @author  Richard Evans, rfle500@york.ac.uk, Andrea Meo, am1808@york.ac.uk (Revision)
/// @version 1.0
/// @date    10/03/2011
///
/// @return EXIT_SUCCESS
///
/// @internal
///	Created:		28/01/2010
///	Revision:	06/07/2017
///=====================================================================================
///
int static_hysteresis(){

	// check calling of routine if error checking is activated
	if(err::check==true){std::cout << "program::static_hysteresis has been called" << std::endl;}

	// Disable temperature as this will prevent convergence
	sim::temperature = 0.0;
	sim::hamiltonian_simulation_flags[3] = 0;	// Thermal

	// Setup min and max fields and increment (uT)
	int iHmax=vmath::iround(double(sim::Hmax)*1.0E6);
	int miHmax=-iHmax;
	int parity_old;
	int iH_old;
	int start_time;

	// Equilibrate system in saturation field, i.e. the largest between equilibration and maximum field set by the user
   if(sim::Heq >= sim::Hmax){
	   sim::H_applied=sim::Heq;
   }
   else{
   	sim::H_applied=sim::Hmax;
   }

	// Initialise sim::integrate only if it not a checkpoint
	if(sim::load_checkpoint_flag && sim::load_checkpoint_continue_flag){}
	else sim::integrate(sim::equilibration_time);

   // Hinc must be positive
	int iHinc=vmath::iround(double(fabs(sim::Hinc))*1.0E6);

   int Hfield;
   int iparity=sim::parity;
	parity_old=iparity;

   // Save value of iH from previous simulation
	if(sim::load_checkpoint_continue_flag) iH_old=int(sim::iH);

	// Perform Field Loop -parity
	while(iparity<2){
      // If checkpoint is loaded with continue flag, then set up correctly max,min field values
		if(sim::load_checkpoint_flag && sim::load_checkpoint_continue_flag)
      {
         //necessary to upload value of iH_old when loading the checkpoint !!!
		   iH_old=int(sim::iH);
         //Setup min and max fields and increment (uT)
			if(parity_old<0){
				if(iparity<0) miHmax=iH_old;
				else if(iparity>0 && iH_old<=0) miHmax=iH_old; //miHmax=(iHmax-iHinc);
				else if(iparity>0 && iH_old>0) miHmax=-(iHmax);
			}
			else if(parity_old>0) miHmax=iH_old;
			Hfield=miHmax;
		}
		else	Hfield=miHmax;

		// Perform Field Loop -field
		while(Hfield<=iHmax){

			// Set applied field (Tesla)
			sim::H_applied=double(Hfield)*double(iparity)*1.0e-6;

			// Reset start time
			start_time=sim::time;

			// Reset mean magnetisation counters
			stats::mag_m_reset();

			// Integrate system
			while(sim::time<sim::loop_time+start_time){

				// Integrate system
				sim::integrate(sim::partial_time);

				double torque=stats::max_torque(); // needs correcting for new integrators
				if((torque<1.0e-6) && (sim::time-start_time>100)){
					break;
				}

				// Calculate mag_m, mag
				stats::mag_m();

         } // End integration loop

			// Increment of iH
			Hfield+=iHinc;
			sim::iH=int64_t(Hfield); //sim::iH+=iHinc;


			// Output to screen and file after each field
			vout::data();

		} // End of field loop

		// Increment of parity
      iparity+=2;
      sim::parity=int64_t(iparity);

	} // End of parity loop

	return EXIT_SUCCESS;
} // end of static-hysteresis-loop

}//end of namespace program
