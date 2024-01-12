//**************************************************
// \file HGCALTBEventAction.cc
// \brief: Implementation of HGCALTBEventAction
//         class
// \author: Lorenzo Pezzotti (CERN EP-SFT-sim)
//          @lopezzot
// \start date: 11 January 2024
//**************************************************

// Includers from project files
//
#include "HGCALTBEventAction.hh"

#include "HGCALTBCEESD.hh"
#include "HGCALTBCHESD.hh"
#include "HGCALTBRunAction.hh"

// Includers from Geant4
//
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4Version.hh"
#if G4VERSION_NUMBER < 1100
#  include "g4root.hh"
#else
#  include "G4AnalysisManager.hh"
#endif
#include "G4SDManager.hh"

// Includers from std
//
#include <numeric>

// constructor and de-constructor
//
HGCALTBEventAction::HGCALTBEventAction() : G4UserEventAction(), edep(0.)
{
  fCEELayerSignals = std::vector<G4double>(HGCALTBConstants::CEELayers, 0.);
  fCHELayerSignals = std::vector<G4double>(HGCALTBConstants::CHELayers, 0.);
}

HGCALTBEventAction::~HGCALTBEventAction() {}

// Define BeginOfEventAction() and EndOfEventAction() virtual methods
//
void HGCALTBEventAction::BeginOfEventAction(const G4Event*)
{
  // Initialize variables per event
  //
  edep = 0.;
  for (auto& value : fCEELayerSignals) {
    value = 0.;
  }
  for (auto& value : fCHELayerSignals) {
    value = 0.;
  }
}

// GetCEEHitsCollection method()
//
HGCALTBCEEHitsCollection* HGCALTBEventAction::GetCEEHitsCollection(G4int hcID,
                                                                   const G4Event* event) const
{
  auto hitsCollection =
    static_cast<HGCALTBCEEHitsCollection*>(event->GetHCofThisEvent()->GetHC(hcID));

  if (!hitsCollection) {
    G4ExceptionDescription msg;
    msg << "Cannot access hitsCollection ID " << hcID;
    G4Exception("HGCALTBEventAction::GetHitsCollection()", "MyCode0003", FatalException, msg);
  }

  return hitsCollection;
}

// GetCEEHitsCollection method()
//
HGCALTBCHEHitsCollection* HGCALTBEventAction::GetCHEHitsCollection(G4int hcID,
                                                                   const G4Event* event) const
{
  auto hitsCollection =
    static_cast<HGCALTBCHEHitsCollection*>(event->GetHCofThisEvent()->GetHC(hcID));

  if (!hitsCollection) {
    G4ExceptionDescription msg;
    msg << "Cannot access hitsCollection ID " << hcID;
    G4Exception("HGCALTBEventAction::GetHitsCollection()", "MyCode0003", FatalException, msg);
  }

  return hitsCollection;
}

void HGCALTBEventAction::EndOfEventAction(const G4Event* event)
{
  // Access Event random seeds
  //
  // auto rndseed = G4RunManager::GetRunManager()->GetRandomNumberStatusForThisEvent();

  // CEE Hits
  //
  auto CEEHCID =
    G4SDManager::GetSDMpointer()->GetCollectionID(HGCALTBCEESD::fCEEHitsCollectionName);
  HGCALTBCEEHitsCollection* CEEHC = GetCEEHitsCollection(CEEHCID, event);

  for (std::size_t i = 0; i < HGCALTBConstants::CEELayers; i++) {
    auto CEESignals = (*CEEHC)[i]->GetCEESignals();
    G4double CEELayerSignal = std::accumulate(CEESignals.begin(), CEESignals.end(), 0.);
    fCEELayerSignals[i] = CEELayerSignal / HGCALTBConstants::MIPSilicon;
  }

  // CHE Hits
  //
  auto CHEHCID =
    G4SDManager::GetSDMpointer()->GetCollectionID(HGCALTBCHESD::fCHEHitsCollectionName);
  HGCALTBCHEHitsCollection* CHEHC = GetCHEHitsCollection(CHEHCID, event);

  for (std::size_t i = 0; i < HGCALTBConstants::CHELayers; i++) {
    auto CHESignals = (*CHEHC)[i]->GetCHESignals();
    G4double CHELayerSignal = std::accumulate(CHESignals.begin(), CHESignals.end(), 0.);
    fCHELayerSignals[i] = CHELayerSignal / HGCALTBConstants::MIPSilicon;
  }

  // Accumulate statistics
  //
  auto CEETot = std::accumulate(fCEELayerSignals.begin(), fCEELayerSignals.end(), 0.);
  auto CHETot = std::accumulate(fCHELayerSignals.begin(), fCHELayerSignals.end(), 0.);
  auto HGCALTot = CEETot + CHETot;
  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->FillNtupleDColumn(0, edep);
  analysisManager->FillNtupleDColumn(1, CEETot);
  analysisManager->FillNtupleDColumn(2, CHETot);
  analysisManager->FillNtupleDColumn(3, HGCALTot);
  analysisManager->AddNtupleRow();
}

//**************************************************
