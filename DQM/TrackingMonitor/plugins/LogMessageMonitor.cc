// -*- C++ -*-
//
// Package:    LogMessageMonitor
// Class:      LogMessageMonitor
// 
/**\class LogMessageMonitor LogMessageMonitor.cc DQM/LogMonitor/src/LogMessageMonitor.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Mia Tosi,40 3-B32,+41227671609,
//         Created:  Thu Mar  8 14:34:13 CET 2012
// $Id$
//
//

#include "DQM/TrackingMonitor/plugins/LogMessageMonitor.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "DataFormats/Common/interface/Handle.h"

#include "FWCore/MessageLogger/interface/ELseverityLevel.h"
#include "FWCore/MessageLogger/interface/ELstring.h"
#include "FWCore/MessageLogger/interface/ErrorSummaryEntry.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DQM/TrackingMonitor/interface/GetLumi.h"
#include "CommonTools/TriggerUtils/interface/GenericTriggerEventFlag.h"
//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
LogMessageMonitor::LogMessageMonitor(const edm::ParameterSet& iConfig)
  : dqmStore_( edm::Service<DQMStore>().operator->() )
  , conf_ ( iConfig )
  , modules_vector_  ( iConfig.getParameter<std::vector<std::string> >("modules")         )
  , doWarningsPlots_ ( iConfig.getParameter<bool>                     ("doWarningsPlots") )
{
   //now do what ever initialization is needed
  lumiDetails_         = new GetLumi( iConfig.getParameter<edm::ParameterSet>("BXlumiSetup") ); 
  //  genTriggerEventFlag_ = new GenericTriggerEventFlag(iConfig);
}


LogMessageMonitor::~LogMessageMonitor()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
  //  if ( lumiDetails_         ) delete lumiDetails_;
  //  if ( genTriggerEventFlag_ ) delete genTriggerEventFlag_;

}


//
// member functions
//

// ------------ method called for each event  ------------
void
LogMessageMonitor::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  // Filter out events if Trigger Filtering is requested
  //  if (genTriggerEventFlag_->on()&& ! genTriggerEventFlag_->accept( iEvent, iSetup) ) return;

  double BXlumi = lumiDetails_->getValue(iEvent);
  
  // Take the ErrorSummaryEntry container
  edm::Handle<std::vector<edm::ErrorSummaryEntry> >  errors;
  iEvent.getByLabel("logErrorHarvester",errors);
  // Check that errors is valid
  if(!errors.isValid()) return; 
  // Compare severity level of error with ELseveritylevel instance el : "-e" should be the lowest error
  edm::ELseverityLevel el("-e");
  
  // Find the total number of errors in iEvent
  if(errors->size()==0){
    for(size_t i = 0; i < modulesMap.size(); i++) {
      ModulesErrorsVsBXlumi[i] -> Fill(BXlumi,0.);
      if ( doWarningsPlots_ )
	ModulesWarningsVsBXlumi[i] -> Fill(BXlumi,0.);
    }
  } else {
    for( size_t i = 0, n = errors->size();
	i < n ; i++){    
      
      //      std::cout << "Severity for error/warning: " << (*errors)[i].severity << " " <<(*errors)[i].module  << std::endl;
      
      // remove the first part of the module string, what is before ":"
      std::string s = (*errors)[i].module;
      //      std::cout << "s: " << s << std::endl;
      size_t pos = s.find(':');
      std::string s_temp = s.substr(pos+1,s.size());
      std::map<std::string,int>::const_iterator it = modulesMap.find(s_temp);
      //      std::cout << "it: " << " --> " << s_temp << std::endl;
      if (it!=modulesMap.end()){
	// IF THIS IS AN ERROR on the ELseverityLevel SCALE, FILL ERROR HISTS
	if((*errors)[i].severity.getLevel() >= el.getLevel()){
	  //      if (categoryECount.size()<=40)
	  //        categoryECount[(*errors)[i].category]+=(*errors)[i].count;
	  //       std::map<std::string,int>::const_iterator it = modulesMap.find((*errors)[i].category);
	  //	  std::cout << "it->second: " << it->second << std::endl;
	  ModulesErrorsVsBXlumi[it->second]->Fill (BXlumi, (*errors)[i].count);
	} else {
	  // IF ONLY WARNING, FILL WARNING HISTS
	  if ( doWarningsPlots_ ) 
	    ModulesWarningsVsBXlumi[it->second]->Fill(BXlumi, (*errors)[i].count);
	}
      }
    }
  }
}


// ------------ method called once each job just before starting event loop  ------------
void 
LogMessageMonitor::beginJob()
{
   // MAKE MODULEMAP USING INPUT FROM CFG FILE
   for (size_t i = 0; i < modules_vector_.size(); i++){
     modulesMap.insert( std::pair<std::string,int>(modules_vector_[i],i) );
   }

   // BOOK THE HISTOGRAMS
   // get binning from the configuration
   edm::ParameterSet BXlumiParameters = conf_.getParameter<edm::ParameterSet>("BXlumiSetup");
   int    BXlumiBin   = BXlumiParameters.getParameter<int>("BXlumiBin");
   double BXlumiMin   = BXlumiParameters.getParameter<double>("BXlumiMin");
   double BXlumiMax   = BXlumiParameters.getParameter<double>("BXlumiMax");
//   int    BXlumiBin   = conf_.getParameter<int>("BXlumiBin");
//   double BXlumiMin   = conf_.getParameter<double>("BXlumiMin");
//   double BXlumiMax   = conf_.getParameter<double>("BXlumiMax");
   
   std::string MEFolderName = conf_.getParameter<std::string>("LogFolderName"); 

   size_t i = 0;
   for(std::map<std::string,int>::const_iterator it = modulesMap.begin();
       it != modulesMap.end(); ++it, i++){ 

     dqmStore_->setCurrentFolder(MEFolderName + "/PUmonitoring/Errors");      

     histname = "errorsVsBXlumi_" + it->first;
     ModulesErrorsVsBXlumi.push_back( dynamic_cast<MonitorElement*>(dqmStore_->bookProfile( histname, histname, BXlumiBin, BXlumiMin, BXlumiMax, 0.,100, "")) );
     ModulesErrorsVsBXlumi[i] -> setAxisTitle("BXlumi [10^{30} Hz cm^{-2}]", 1);
     ModulesErrorsVsBXlumi[i] -> setAxisTitle("Mean number of errors", 2);
     
     if ( doWarningsPlots_ ) {
       dqmStore_->setCurrentFolder(MEFolderName + "/PUmonitoring/Warnings");      
       
       histname = "warningVsBXlumi_" + it->first;
       ModulesWarningsVsBXlumi.push_back( dynamic_cast<MonitorElement*>(dqmStore_->bookProfile( histname, histname, BXlumiBin, BXlumiMin, BXlumiMax, 0.,100, "")) );
       ModulesWarningsVsBXlumi[i] -> setAxisTitle("BXlumi [10^{30} Hz cm^{-2}]", 1);
       ModulesWarningsVsBXlumi[i] -> setAxisTitle("Mean number of warnings", 2);
     }
   }   
}

// ------------ method called once each job just after ending the event loop  ------------
void 
LogMessageMonitor::endJob() 
{
    bool outputMEsInRootFile   = conf_.getParameter<bool>("OutputMEsInRootFile");
    std::string outputFileName = conf_.getParameter<std::string>("OutputFileName");
    if(outputMEsInRootFile)
    {
        dqmStore_->showDirStructure();
        dqmStore_->save(outputFileName);
    }
}

// ------------ method called when starting to processes a run  ------------
void 
LogMessageMonitor::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup)
{
  //  if ( genTriggerEventFlag_->on() ) genTriggerEventFlag_->initRun( iRun, iSetup );
}

// ------------ method called when ending the processing of a run  ------------
void 
LogMessageMonitor::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
LogMessageMonitor::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
LogMessageMonitor::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
LogMessageMonitor::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(LogMessageMonitor);
