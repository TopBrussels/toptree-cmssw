#ifndef DataRecord_DQMSummaryRcd_h
#define DataRecord_DQMSummaryRcd_h
// -*- C++ -*-
//
// Package:     DataRecord
// Class  :     DQMSummaryRcd
// 
/**\class DQMSummaryRcd DQMSummaryRcd.h CondFormats/DataRecord/interface/DQMSummaryRcd.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Author:      Salvatore di Guida (CERN)
// Created:     Wed Mar 18 15:11:00 CET 2009
// $Id: DQMSummaryRcd.h,v 1.1 2009/03/27 16:04:31 emanuele Exp $
//

#include "FWCore/Framework/interface/EventSetupRecordImplementation.h"

class DQMSummaryRcd : public edm::eventsetup::EventSetupRecordImplementation<DQMSummaryRcd> {};

#endif
