/*  DYNAMO:- Event driven molecular dynamics simulator 
    http://www.marcusbannerman.co.uk/dynamo
    Copyright (C) 2009  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

    This program is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 3 as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "misc.hpp"
#include <boost/foreach.hpp>
#include "../../extcode/xmlwriter.hpp"
#include "../../dynamics/include.hpp"
#include "../../base/is_simdata.hpp"
#include "../../datatypes/vector.xml.hpp"
#include <ctime>
#include <sys/time.h>
#include <sys/resource.h>
#include "../../extcode/memUsage.hpp"

OPMisc::OPMisc(const DYNAMO::SimData* tmp, const XMLNode&):
  OutputPlugin(tmp,"Misc",0),
  oldSysTime(0),
  dualEvents(0),
  singleEvents(0),
  oldcoll(0)
{}

void 
OPMisc::changeSystem(OutputPlugin* misc2)
{
  std::swap(Sim, static_cast<OPMisc*>(misc2)->Sim);
}

void
OPMisc::initialise()
{
  Iflt kt = Sim->dynamics.getLiouvillean().getkT();

  Vector  VecEnergy(Sim->dynamics.getLiouvillean().getVectorSystemKineticEnergy());
  
  VecEnergy *= 2.0 / (Sim->lN * Sim->dynamics.units().unitEnergy());

  I_cout() << "Particle Count " << Sim->lN 
	   << "\nSim Unit Length " << Sim->dynamics.units().unitLength()
	   << "\nSim Unit Time " << Sim->dynamics.units().unitTime()
	   << "\nDensity " << Sim->dynamics.getNumberDensity() 
    * Sim->dynamics.units().unitVolume()
	   << "\nPacking Fraction " << Sim->dynamics.getPackingFraction()
	   << "\nSim Temperature " << kt
	   << "\nReduced Temperature " << kt / Sim->dynamics.units().unitEnergy();
  
  for (size_t iDim(0); iDim < NDIM; ++iDim)
    I_cout() << "Kinetic Temperature dimension" << iDim << " " 
	     <<  VecEnergy[iDim];

  I_cout() << "No. of Species " << Sim->dynamics.getSpecies().size()
      << "\nSimulation box length <x,y,z> ";
  for (size_t iDim = 0; iDim < NDIM; iDim++)
    std::cout  << Sim->aspectRatio[iDim]/Sim->dynamics.units().unitLength() << " ";
  
  Vector  sumMV (0,0,0);
  
  //Determine the discrepancy VECTOR
  BOOST_FOREACH( const CParticle & Part, Sim->vParticleList)
    {
      Vector  pos(Part.getPosition()), vel(Part.getVelocity());
      Sim->dynamics.BCs().applyBC(pos, vel);

      sumMV += vel * Sim->dynamics.getSpecies(Part).getMass();
    }
  
  I_cout() << "Total momentum <x,y,z> <";

  for (size_t iDim = 0; iDim < NDIM; iDim++)
    std::cout  << sumMV[iDim] / Sim->dynamics.units().unitMomentum() << " ";

  std::cout << ">";

  std::time(&tstartTime);
  clock_gettime(CLOCK_MONOTONIC, &acc_tstartTime);

  std::string sTime(std::ctime(&tstartTime));
  sTime[sTime.size()-1] = ' ';

  I_cout() << "Started on " << sTime;
}

void 
OPMisc::eventUpdate(const CIntEvent&, const C2ParticleData&) 
{
  dualEvents++;
}

void 
OPMisc::eventUpdate(const CGlobEvent&, const CNParticleData& NDat)
{
  dualEvents += NDat.L2partChanges.size();
  singleEvents += NDat.L1partChanges.size();
}

void 
OPMisc::eventUpdate(const CLocalEvent&, const CNParticleData& NDat)
{
  dualEvents += NDat.L2partChanges.size();
  singleEvents += NDat.L1partChanges.size();
}

void 
OPMisc::eventUpdate(const CSystem&, const CNParticleData& NDat, 
		     const Iflt&)
{
  dualEvents += NDat.L2partChanges.size();
  singleEvents += NDat.L1partChanges.size();
}

Iflt
OPMisc::getMFT() const
{
  return Sim->dSysTime * static_cast<Iflt>(Sim->lN)
    /(Sim->dynamics.units().unitTime() 
      * ((2.0 * static_cast<Iflt>(dualEvents)) 
	 + static_cast<Iflt>(singleEvents)));
}


void
OPMisc::output(xmlw::XmlStream &XML)
{
  timespec acc_tendTime;
  clock_gettime(CLOCK_MONOTONIC, &acc_tendTime);

  std::time_t tendTime;
  time(&tendTime);
  
  std::string sTime(std::ctime(&tstartTime));
  //A hack to remove the newline character at the end
  sTime[sTime.size()-1] = ' ';

  std::string eTime(std::ctime(&tendTime));
  //A hack to remove the newline character at the end
  eTime[eTime.size()-1] = ' ';
  
  Iflt collpersec = static_cast<Iflt>(Sim->lNColl)
    / (Iflt(acc_tendTime.tv_sec) + 1e-9 * Iflt(acc_tendTime.tv_nsec)
       - Iflt(acc_tstartTime.tv_sec) - 1e-9 * Iflt(acc_tstartTime.tv_nsec));

  long int maxmemusage;

  {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    maxmemusage = ru.ru_maxrss;
  }

  I_cout() << "Ended on " << eTime
	   << "\nTotal Collisions Executed " << Sim->lNColl
	   << "\nAvg Coll/s " << collpersec
	   << "\nSim time per second " 
	   << Sim->dSysTime / (Sim->dynamics.units().unitTime() 
			       * static_cast<Iflt>(tendTime - tstartTime));
   
  XML << xmlw::tag("Misc")
      << xmlw::tag("Memusage")
      << xmlw::attr("MaxKiloBytes") << maxmemusage
      << xmlw::endtag("Memusage")
      << xmlw::tag("Density")
      << xmlw::attr("val") << Sim->dynamics.getNumberDensity() * Sim->dynamics.units().unitVolume()
      << xmlw::endtag("Density")
    
      << xmlw::tag("PackingFraction")
      << xmlw::attr("val") << Sim->dynamics.getPackingFraction()
      << xmlw::endtag("PackingFraction")

      << xmlw::tag("SpeciesCount")
      << xmlw::attr("val") << Sim->dynamics.getSpecies().size()
      << xmlw::endtag("SpeciesCount")
    
      << xmlw::tag("ParticleCount")
      << xmlw::attr("val") << Sim->lN
      << xmlw::endtag("ParticleCount")
    
      << xmlw::tag("SimLength")
      << xmlw::attr("Collisions") << Sim->lNColl
      << xmlw::attr("Time") << Sim->dSysTime / Sim->dynamics.units().unitTime()
      << xmlw::endtag("SimLength")
    
      << xmlw::tag("Timing") 

      << xmlw::tag("Start") 
      << xmlw::attr("val") << sTime
      << xmlw::endtag("Start")
  
      << xmlw::tag("End") 
      << xmlw::attr("val") << eTime
      << xmlw::endtag("End")

      << xmlw::tag("Duration") 
      << xmlw::attr("val") 
      << tendTime - tstartTime
      << xmlw::endtag("Duration")

      << xmlw::tag("CollPerSec")
      << xmlw::attr("val") << collpersec
      << xmlw::endtag("CollPerSec")

      << xmlw::endtag("Timing") 
      << xmlw::tag("SystemBoxLength")
      << xmlw::attr("val") 
      << 1.0/Sim->dynamics.units().unitLength();

  char name[2] = "x";
  for (size_t iDim = 0; iDim < NDIM; iDim++)
    {
      name[0] = 'x' + iDim;
      XML << xmlw::tag(name) << xmlw::attr("val")
	  << Sim->aspectRatio[iDim]/Sim->dynamics.units().unitLength() 
	  << xmlw::endtag(name);
    }
  
  XML << xmlw::endtag("SystemBoxLength");

  Vector  sumMV (0,0,0);
  
  //Determine the discrepancy VECTOR
  BOOST_FOREACH( const CParticle & Part, Sim->vParticleList)
    sumMV += Part.getVelocity() * Sim->dynamics.getSpecies(Part).getMass();
  
  XML << xmlw::tag("Total_momentum") 
      << sumMV / Sim->dynamics.units().unitMomentum()
      << xmlw::endtag("Total_momentum")    
      << xmlw::tag("totMeanFreeTime")
      << xmlw::attr("val")
      << getMFT()
      << xmlw::endtag("totMeanFreeTime");
  
  std::pair<Iflt, Iflt> mempair = process_mem_usage();
  
  XML << xmlw::tag("MemoryUsage")
      << xmlw::attr("VirtualMemory") << mempair.first
      << xmlw::attr("ResidentSet") << mempair.second
      << xmlw::endtag("MemoryUsage")
      << xmlw::endtag("Misc");
}

void
OPMisc::periodicOutput()
{
  time_t rawtime;
  time(&rawtime);

  tm timeInfo;
  localtime_r (&rawtime, &timeInfo);

  char dateString[12] = "";
  strftime(dateString, 12, "%a %H:%M |", &timeInfo);

  I_Pcout() << dateString << " NColls " << (Sim->lNColl+1)/1000 << "k, t "
	    << Sim->dSysTime/Sim->dynamics.units().unitTime() << ", <t_2> "
	    <<   Sim->dSysTime * static_cast<Iflt>(Sim->lN)
    /(Sim->dynamics.units().unitTime() * 2.0 * static_cast<Iflt>(dualEvents))
	    << ", <t_tot> "
	    <<   Sim->dSysTime * static_cast<Iflt>(Sim->lN)
    / (Sim->dynamics.units().unitTime() * (2.0 * static_cast<Iflt>(dualEvents) 
					   + static_cast<Iflt>(singleEvents)))
	    << ", "; 

  oldSysTime = Sim->dSysTime;
  oldcoll = Sim->lNColl;
}
