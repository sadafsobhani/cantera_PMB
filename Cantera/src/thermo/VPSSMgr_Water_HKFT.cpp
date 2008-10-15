/**
 *  @file VPSSMgr_Water_HKFT.cpp
 * Definition file for a derived class that handles the calculation
 * of standard state thermo properties for pure water and
 *  a set of species which obey the HKFT standard state
 * dependence 
 * (see \ref thermoprops and class 
 * \link Cantera::VPSSMgr_Water_HKFT VPSSMgr_Water_HKFT\endlink).
 */
/*
 * Copywrite (2005) Sandia Corporation. Under the terms of 
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */
/*
 *  $Author$
 *  $Date$
 *  $Revision$
 */

// turn off warnings under Windows
#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include "VPSSMgr_Water_HKFT.h"
#include "xml.h"
#include "VPStandardStateTP.h"
#include "PDSS_Water.h"
#include "PDSS_HKFT.h"

using namespace std;

namespace Cantera {

  VPSSMgr_Water_HKFT::VPSSMgr_Water_HKFT(VPStandardStateTP *vp_ptr,
					 SpeciesThermo *spth) :
    VPSSMgr(vp_ptr, spth),
    m_waterSS(0)
  {
    m_useTmpRefStateStorage      = true;
    m_useTmpStandardStateStorage = true;
  }


  VPSSMgr_Water_HKFT::~VPSSMgr_Water_HKFT() 
  {
    //  m_waterSS is owned by VPStandardState
  }

  VPSSMgr_Water_HKFT::VPSSMgr_Water_HKFT(const VPSSMgr_Water_HKFT &right) :
    VPSSMgr(right.m_vptp_ptr, right.m_spthermo),
    m_waterSS(0)
  {
    m_useTmpRefStateStorage = true;
    m_useTmpStandardStateStorage = true;
    *this = right;
  }


  VPSSMgr_Water_HKFT& 
  VPSSMgr_Water_HKFT::operator=(const VPSSMgr_Water_HKFT &b) 
  {
    if (&b == this) return *this;
    VPSSMgr::operator=(b);
    m_waterSS = (PDSS_Water *) m_vptp_ptr->providePDSS(0);
    return *this;
  }

  VPSSMgr *
  VPSSMgr_Water_HKFT::duplMyselfAsVPSSMgr() const {
    VPSSMgr_Water_HKFT *vpm = new VPSSMgr_Water_HKFT(*this);
    return (VPSSMgr *) vpm;
  }

  void
  VPSSMgr_Water_HKFT::getEnthalpy_RT_ref(doublereal *hrt) const{
    // Everything should be OK except for the water SS
    if (m_p0 != m_plast) {
      doublereal RT = GasConstant * m_tlast;
      m_waterSS->setState_TP(m_tlast, m_p0);
      m_h0_RT[0] = (m_waterSS->enthalpy_mole()) / RT;
      m_waterSS->setState_TP(m_tlast, m_plast);
    } else {
      m_h0_RT[0] = m_hss_RT[0];
    }
    copy(m_h0_RT.begin(), m_h0_RT.end(), hrt);
  }

  void
  VPSSMgr_Water_HKFT::getGibbs_RT_ref(doublereal *grt) const{
    // Everything should be OK except for the water SS
    if (m_p0 != m_plast) {
      doublereal RT = GasConstant * m_tlast;
      m_waterSS->setState_TP(m_tlast, m_p0);
      m_g0_RT[0] = (m_waterSS->gibbs_mole()) / RT;
      m_waterSS->setState_TP(m_tlast, m_plast);
    } else {
      m_g0_RT[0] = m_gss_RT[0];
    }
    copy(m_g0_RT.begin(), m_g0_RT.end(), grt);
  }

  void 
  VPSSMgr_Water_HKFT::getGibbs_ref(doublereal *g) const{
    doublereal RT = GasConstant * m_tlast;
    for (int k = 0; k < m_kk; k++) {
      g[k] *= RT;
    }
  }

  void
  VPSSMgr_Water_HKFT::getEntropy_R_ref(doublereal *sr) const{
    // Everything should be OK except for the water SS
    if (m_p0 != m_plast) {
      m_waterSS->setState_TP(m_tlast, m_p0);
      m_s0_R[0] = (m_waterSS->entropy_mole()) / GasConstant;
      m_waterSS->setState_TP(m_tlast, m_plast);
    } else {
      m_s0_R[0] = m_sss_R[0];
    }
    copy(m_s0_R.begin(), m_s0_R.end(), sr);
  }

  void
  VPSSMgr_Water_HKFT::getCp_R_ref(doublereal *cpr) const{
    // Everything should be OK except for the water SS
    if (m_p0 != m_plast) {
      m_waterSS->setState_TP(m_tlast, m_p0);
      m_cp0_R[0] = (m_waterSS->cp_mole()) / GasConstant;
      m_waterSS->setState_TP(m_tlast, m_plast);
    } else {
      m_cp0_R[0] = m_cpss_R[0];
    }
    copy(m_cp0_R.begin(), m_cp0_R.end(), cpr);
  }

  void
  VPSSMgr_Water_HKFT::getStandardVolumes_ref(doublereal *vol) const{
    // Everything should be OK except for the water SS
    if (m_p0 != m_plast) {
     m_waterSS->setState_TP(m_tlast, m_p0);
     m_V0[0] = (m_waterSS->density())      / m_vptp_ptr->molecularWeight(0);
     m_waterSS->setState_TP(m_tlast, m_plast);
    } else {
      m_V0[0] = m_Vss[0];
    }
    copy(m_V0.begin(), m_V0.end(), vol);
  }


  void VPSSMgr_Water_HKFT::updateRefStateThermo() const {
    // Fix up the water
    doublereal RT = GasConstant * m_tlast;
    m_waterSS->setState_TP(m_tlast, m_p0);
    m_h0_RT[0] = (m_waterSS->enthalpy_mole())/ RT;
    m_s0_R[0]  = (m_waterSS->entropy_mole()) / GasConstant;
    m_cp0_R[0] = (m_waterSS->cp_mole()) / GasConstant;
    m_g0_RT[0] = (m_hss_RT[0] - m_sss_R[0]);
    m_V0[0]    = (m_waterSS->density())      / m_vptp_ptr->molecularWeight(0);
    m_waterSS->setState_TP(m_tlast, m_plast);

    for (int k = 1; k < m_kk; k++) {
      PDSS_HKFT *ps = (PDSS_HKFT *) m_vptp_ptr->providePDSS(k);
      ps->setState_TP(m_tlast, m_p0);
      m_cpss_R[k]  = ps->cp_R();
      m_sss_R[k]   = ps->entropy_mole();
      m_gss_RT[k]  = ps->gibbs_RT();;
      m_hss_RT[k]  = m_gss_RT[k] + m_sss_R[k];
      m_Vss[k]     = ps->molarVolume();
    }
  }

  void VPSSMgr_Water_HKFT::_updateStandardStateThermo() {
    doublereal RT = GasConstant * m_tlast;
    // Do the water
    m_waterSS->setState_TP(m_tlast, m_plast);
    m_hss_RT[0] = (m_waterSS->enthalpy_mole())/ RT;
    m_sss_R[0]  = (m_waterSS->entropy_mole()) / GasConstant;
    m_cpss_R[0] = (m_waterSS->cp_mole())      / GasConstant;
    m_gss_RT[0] = (m_hss_RT[0] - m_sss_R[0]);
    m_Vss[0]    = (m_vptp_ptr->molecularWeight(0)) / (m_waterSS->density());

    for (int k = 1; k < m_kk; k++) {
      PDSS_HKFT *ps = (PDSS_HKFT *) m_vptp_ptr->providePDSS(k);
      ps->setState_TP(m_tlast, m_plast);
      m_cpss_R[k]  = ps->cp_R();
      m_sss_R[k]   = ps->entropy_R();
      m_gss_RT[k]  = ps->gibbs_RT();;
      m_hss_RT[k]  = m_gss_RT[k] + m_sss_R[k];
      m_Vss[k]     = ps->molarVolume();
    }
 
  }

  void VPSSMgr_Water_HKFT::initThermo() {
    VPSSMgr::initThermo();
  }


  void 
  VPSSMgr_Water_HKFT::initThermoXML(XML_Node& phaseNode, std::string id) {
    VPSSMgr::initThermoXML(phaseNode, id);
   
    XML_Node& speciesList = phaseNode.child("speciesArray");
    XML_Node* speciesDB = get_XML_NameID("speciesData", speciesList["datasrc"],
					 &phaseNode.root());
    const vector<string>&sss = m_vptp_ptr->speciesNames();

    m_waterSS->setState_TP(300., OneAtm);
    m_Vss[0] =  (m_waterSS->density())      / m_vptp_ptr->molecularWeight(0);

    for (int k = 1; k < m_kk; k++) {
      const XML_Node* s =  speciesDB->findByAttr("name", sss[k]);
      if (!s) {
	throw CanteraError("VPSSMgr_Water_HKFT::initThermoXML",
			   "no species Node for species " + sss[k]);
      }
      const XML_Node *ss = s->findByName("standardState");
      if (!ss) {
	throw CanteraError("VPSSMgr_Water_HKFT::initThermoXML",
			   "no standardState Node for species " + s->name());
      }
      std::string model = lowercase((*ss)["model"]);
      if (model != "hkft") {
	throw CanteraError("VPSSMgr_Water_HKFT::initThermoXML",
			   "standardState model for species isn't hkft: " + s->name());
      }
    }   
  }

  PDSS *
  VPSSMgr_Water_HKFT::createInstallPDSS(int k, const XML_Node& speciesNode,  
					const XML_Node *phaseNode_ptr) {
   PDSS *kPDSS = 0;

    const XML_Node *ss = speciesNode.findByName("standardState");
    if (!ss) {
      throw CanteraError("VPSSMgr_Water_HKFT::installSpecies",
			 "no standardState Node for species " + speciesNode.name());
    }
    // Will have to do something for water 
    // -> make sure it's species 0
    // -> make sure it's designated as a real water EOS
    if (k == 0) {
      string xn = speciesNode["name"];
      if (xn != "H2O(L)") {
	throw CanteraError("VPSSMgr_Water_HKFT::installSpecies",
			   "h2o wrong name: " + xn);
      }
 
      std::string model = (*ss)["model"];
      if (model != "waterIAPWS" && model != "waterPDSS") {
	throw CanteraError("VPSSMgr_Water_HKFT::installSpecies",
			   "wrong SS mode: " + model);
      }
      VPSSMgr::installSTSpecies(k, speciesNode, phaseNode_ptr);
      if (m_waterSS) delete m_waterSS;
      m_waterSS = new PDSS_Water(m_vptp_ptr, 0);
      kPDSS = m_waterSS;
    } else {
      std::string model = (*ss)["model"];
      if (model != "HKFT") {
	throw CanteraError("VPSSMgr_Water_HKFT::initThermoXML",
			   "standardState model for species isn't "
			   "HKFT: " + speciesNode.name());
      }

      kPDSS = new PDSS_HKFT(m_vptp_ptr, k, speciesNode, *phaseNode_ptr, true);

    }
    return kPDSS;
  }

  PDSS_enumType VPSSMgr_Water_HKFT::reportPDSSType(int k) const {
    return cPDSS_UNDEF;
  }

  VPSSMgr_enumType VPSSMgr_Water_HKFT::reportVPSSMgrType() const {
    return cVPSSMGR_WATER_HKFT;
  }
}

