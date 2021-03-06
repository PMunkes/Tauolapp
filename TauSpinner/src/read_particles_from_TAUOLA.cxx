#include "TauSpinner/read_particles_from_TAUOLA.h"
using namespace TauSpinner;

namespace TauSpinner {

// Global variables - used to store the state of the search
// for boson in 'readParticlesFromTAUOLA_HepMC'
HepMC::GenEvent                          *event = new HepMC::GenEvent();
HepMC::GenEvent::particle_const_iterator     it = event->particles_end();

/*******************************************************************************
  Get HepMC::Event

  Return last event parsed by readParticlesFromTAUOLA_HepMC
*******************************************************************************/
HepMC::GenEvent* readParticlesFromTAUOLA_HepMC_getEvent()
{
  return event;
}

/*******************************************************************************
  Get daughters of HepMC::GenParticle

  Recursively searches for final-state daughters of 'x'
*******************************************************************************/
inline vector<SimpleParticle> *getDaughters(HepMC::GenParticle *x)
{
  vector<SimpleParticle> *daughters = new vector<SimpleParticle>();
  if(!x->end_vertex()) return daughters;

  // Check decay products of 'x'
  for(HepMC::GenVertex::particles_out_const_iterator p = x->end_vertex()->particles_out_const_begin(); p!=x->end_vertex()->particles_out_const_end(); ++p)
  {
    HepMC::GenParticle *pp = *p;
    HepMC::FourVector   mm = pp->momentum();

    // If the daughter of 'x' has its end vertex - recursively read
    // all of its daughters.
    if( pp->end_vertex() && pp->pdg_id()!=111)
    {
      vector<SimpleParticle> *sub_daughters = getDaughters(pp);
      daughters->insert(daughters->end(),sub_daughters->begin(),sub_daughters->end());
      
      delete sub_daughters;
    }
    // Otherwise - add this particle to the list of daughters.
    else
    {
      SimpleParticle tp( mm.px(), mm.py(), mm.pz(), mm.e(), pp->pdg_id() );
      daughters->push_back(tp);
    }
  }

  return daughters;
}

/*******************************************************************************
  Read HepMC::IO_GenEvent.

  Read HepMC event from data file
  and return particles filtered out from the event.
  
  This routine is prepared for use with files generated by Pythia8.
  Fills:
  
  'X'              - Heavy particle (W+/-, H+/-, H, Z)
  'tau'            - first tau
  'tau2'           - second tau or nu_tau, if 'X' decays to one tau only
  'tau_daughters'  - daughters of 'tau'
  'tau2_daughters' - daughters of 'tau2' or empty list, if 'tau2' is nu_tau.
  
  Returns:
  0 - no more events to read               (finished processing the file)
  1 - no decay found in the event          (finished processing current event)
  2 - decay found and processed correctly.
      Event will continue to be processed
      with next function call. 
*******************************************************************************/
int readParticlesFromTAUOLA_HepMC(HepMC::IO_GenEvent &input_file, SimpleParticle &X, SimpleParticle &tau, SimpleParticle &tau2, vector<SimpleParticle> &tau_daughters, vector<SimpleParticle> &tau2_daughters)
{
  // If finished processing the event with previous function call
  if(it == event->particles_end())
  {
    // Delete previous event and create a new one
    delete event;
    event = new HepMC::GenEvent();
    
    // Read (parsed i.e. consiting of W and tau decay vertices only) event from input file
    input_file.fill_next_event(event);
    
    // Finish if there are no more events to read
    if(input_file.rdstate()) return 0;
    
    it = event->particles_begin();
  }

  // Exctract particles from event
  HepMC::GenParticle *hX=NULL, *hTau=NULL, *hTau2=NULL;

  for(; it!=event->particles_end(); ++it)
  {
    int pdgid = (*it)->pdg_id();
    if(
        (
          abs(pdgid)==37 ||
              pdgid ==25 ||
          abs(pdgid)==24 ||
              pdgid ==23          
        ) &&
        (*it)->end_vertex()->particles_out_size()>1 &&
        (*(*it)->end_vertex()->particles_out_const_begin())->pdg_id()!=pdgid
      )
    {
      hX = *it;

      // Get daughters of X
      HepMC::GenVertex   *x_daughters = (*it)->end_vertex();

      // tau should be the 1st daughter of X
      HepMC::GenParticle *first_daughter  =     *x_daughters->particles_out_const_begin();
      HepMC::GenParticle *second_daughter = *(++(x_daughters->particles_out_const_begin()));

      // ignore all self-decays
      while(first_daughter->end_vertex())
      {
        HepMC::GenParticle *buf = (*first_daughter->end_vertex()->particles_out_const_begin());
        if(buf->pdg_id()==first_daughter->pdg_id()) first_daughter = buf;
        else break;
      }
      while(second_daughter->end_vertex())
      {
        HepMC::GenParticle *buf = (*second_daughter->end_vertex()->particles_out_const_begin());
        if(buf->pdg_id()==second_daughter->pdg_id()) second_daughter = buf;
        else break;
      }
      
      // Check if it is tau and it has decay products
      if( abs(first_daughter->pdg_id())==15 && first_daughter->end_vertex()!=NULL)
      {
         hTau  = first_daughter;
         hTau2 = second_daughter;
         ++it;
         break;
      }
      else if( abs(second_daughter->pdg_id())==15 && second_daughter->end_vertex()!=NULL)
      {
         hTau  = second_daughter;
         hTau2 = first_daughter;
         ++it;
         break;
      }
      else hX = NULL;
    }
  }

  // Boson not found - finished processing the event
  if(!hX) return 1;

  if(!hTau || !hTau2)
  {
    cout<<"ERROR: Something is wrong with event record."<<endl;
    exit(-1);
  }

  // Fill SimpleParticles from HepMC particles
  X.setPx(hX->momentum().px());
  X.setPy(hX->momentum().py());
  X.setPz(hX->momentum().pz());
  X.setE (hX->momentum().e ());
  X.setPdgid(hX->pdg_id());

  tau.setPx(hTau->momentum().px());
  tau.setPy(hTau->momentum().py());
  tau.setPz(hTau->momentum().pz());
  tau.setE (hTau->momentum().e ());
  tau.setPdgid(hTau->pdg_id());

  tau2.setPx(hTau2->momentum().px());
  tau2.setPy(hTau2->momentum().py());
  tau2.setPz(hTau2->momentum().pz());
  tau2.setE (hTau2->momentum().e ());
  tau2.setPdgid(hTau2->pdg_id());

  // Create list of tau daughters
  vector<SimpleParticle> *buf = getDaughters(hTau);
  tau_daughters.clear();
  tau_daughters.insert(tau_daughters.end(),buf->begin(),buf->end());
  
  delete buf;

  // Second particle can be 2nd tau. In that case - read its daughters.
  // Otherwise it is nu_tau~
  buf = getDaughters(hTau2);
  tau2_daughters.clear();
  tau2_daughters.insert(tau2_daughters.end(),buf->begin(),buf->end());
  
  delete buf;

  return 2;
}

} // namespace TauSpinner
