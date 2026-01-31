// TMB exports for unmarked package
// Compiled inline as part of main package library for webR compatibility
//
// This follows the approach used by glmmTMB, avoiding the TMB::compile() step
// that fails during cross-compilation for WebAssembly.

#include <float.h>
#include <TMB.hpp>
#include "TMB/tmb_utils.hpp"
#include "TMB/tmb_pifun.hpp"
#include "TMB/tmb_keyfun.hpp"
#include "TMB/tmb_occu.hpp"
#include "TMB/tmb_pcount.hpp"
#include "TMB/tmb_multinomPois.hpp"
#include "TMB/tmb_distsamp.hpp"
#include "TMB/tmb_gdistremoval.hpp"
#include "TMB/tmb_IDS.hpp"
#include "TMB/tmb_goccu.hpp"
#include "TMB/tmb_colext.hpp"

template<class Type>
Type objective_function<Type>::operator() () {
  DATA_STRING(model);
  if(model == "tmb_occu") {
    return tmb_occu(this);
  } else if(model == "tmb_pcount") {
    return tmb_pcount(this);
  } else if(model == "tmb_multinomPois"){
    return tmb_multinomPois(this);
  } else if(model == "tmb_distsamp"){
    return tmb_distsamp(this);
  } else if(model == "tmb_gdistremoval"){
    return tmb_gdistremoval(this);
  } else if(model == "tmb_IDS"){
    return tmb_IDS(this);
  } else if(model == "tmb_goccu"){
    return tmb_goccu(this);
  } else if(model == "tmb_colext"){
    return tmb_colext(this);
  }
  return 0;
}
