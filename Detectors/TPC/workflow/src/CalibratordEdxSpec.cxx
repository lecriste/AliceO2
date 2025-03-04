// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file CalibratordEdxSpec.cxx
/// \brief Workflow for time based dE/dx calibration.
/// \author Thiago Badaró <thiago.saramela@usp.br>

#include "TPCWorkflow/CalibratordEdxSpec.h"

#include <vector>
#include <memory>

// o2 includes
#include "CCDB/CcdbApi.h"
#include "CCDB/CcdbObjectInfo.h"
#include "CommonUtils/NameConf.h"
#include "DataFormatsTPC/TrackTPC.h"
#include "DataFormatsParameters/GRPObject.h"
#include "DetectorsCalibration/Utils.h"
#include "Framework/Task.h"
#include "Framework/DataProcessorSpec.h"
#include "Framework/ConfigParamRegistry.h"
#include "Framework/CCDBParamSpec.h"
#include "TPCCalibration/CalibratordEdx.h"
#include "TPCWorkflow/ProcessingHelpers.h"
#include "DetectorsBase/GRPGeomHelper.h"
#include "TPCBase/CDBInterface.h"
#include "TPCBase/Utils.h"

using namespace o2::framework;

namespace o2::tpc
{

class CalibratordEdxDevice : public Task
{
 public:
  CalibratordEdxDevice(std::shared_ptr<o2::base::GRPGeomRequest> req, const o2::base::Propagator::MatCorrType matType) : mCCDBRequest(req), mMatType(matType) {}
  void init(framework::InitContext& ic) final
  {
    o2::base::GRPGeomHelper::instance().setRequest(mCCDBRequest);
    const auto slotLengthTF = ic.options().get<uint32_t>("tf-per-slot");
    const auto slotLengthSeconds = ic.options().get<int>("seconds-per-slot");
    const auto maxDelay = ic.options().get<uint32_t>("max-delay");
    const auto minEntries = ic.options().get<int>("min-entries");
    mCalibIntervalExtensionMS = ic.options().get<uint32_t>("calib-interval-extension") * 1000l;

    const auto minEntriesSector = ic.options().get<int>("min-entries-sector");
    const auto minEntries1D = ic.options().get<int>("min-entries-1d");
    const auto minEntries2D = ic.options().get<int>("min-entries-2d");
    const auto fitPasses = ic.options().get<int>("fit-passes");
    const auto fitThreshold = ic.options().get<float>("fit-threshold");
    const auto fitThresholdLowFactor = ic.options().get<float>("fit-threshold-low-factor");

    const auto dEdxBins = ic.options().get<int>("dedxbins");
    const auto mindEdx = ic.options().get<float>("min-dedx");
    const auto maxdEdx = ic.options().get<float>("max-dedx");
    const auto angularBins = ic.options().get<int>("angularbins");
    const auto fitSnp = ic.options().get<bool>("fit-snp");

    const auto dumpData = ic.options().get<bool>("file-dump");
    const auto dumpHistograms = ic.options().get<uint32_t>("dump-histograms");
    const auto trackDebug = ic.options().get<bool>("track-debug");
    const bool makeGaussianFits = !ic.options().get<bool>("disable-gaussian-fits");

    mCalibrator = std::make_unique<tpc::CalibratordEdx>();
    mCalibrator->setHistParams(dEdxBins, mindEdx, maxdEdx, angularBins, fitSnp);
    mCalibrator->setApplyCuts(false);
    mCalibrator->setFitThresholds(minEntriesSector, minEntries1D, minEntries2D);
    mCalibrator->setMinEntries(minEntries);
    mCalibrator->setSlotLength(slotLengthTF);
    mCalibrator->setSlotLengthInSeconds(slotLengthSeconds);
    mCalibrator->setMaxSlotsDelay(maxDelay);
    mCalibrator->setElectronCut({fitThreshold, fitPasses, fitThresholdLowFactor});
    mCalibrator->setMaterialType(mMatType);
    mCalibrator->setDumpHistograms(dumpHistograms);
    mCalibrator->setTrackDebug(trackDebug);
    mCalibrator->setMakeGaussianFits(makeGaussianFits);

    if (dumpData) {
      const auto dumpDataName = ic.options().get<std::string>("file-dump-name");
      mCalibrator->enableDebugOutput(dumpDataName);
    }
  }

  void finaliseCCDB(o2::framework::ConcreteDataMatcher& matcher, void* obj) final
  {
    if (o2::base::GRPGeomHelper::instance().finaliseCCDB(matcher, obj)) {
      return;
    }
    if (matcher == ConcreteDataMatcher("TPC", "TIMEGAIN", 0)) {
      mTimeGain = *(o2::tpc::CalibdEdxCorrection*)obj;
      const auto meanParamTot = mTimeGain.getMeanParams(ChargeType::Tot);
      LOGP(info, "Updating TimeGain with {} dimensions and mean qTot Params {}", mTimeGain.getDims(), utils::elementsToString(meanParamTot));
      return;
    }
  }

  void run(ProcessingContext& pc) final
  {
    o2::base::TFIDInfoHelper::fillTFIDInfo(pc, mCalibrator->getCurrentTFInfo());
    o2::base::GRPGeomHelper::instance().checkUpdates(pc);
    checkUpdates(pc);
    static bool slotLengthSet = false;
    if (!slotLengthSet) {
      // Important, otherwise the call to getSlotForTF below will be wrong, must be called after GRPGeomHelper update to get the correct TF length
      mCalibrator->checkSlotLength();
      slotLengthSet = true;
    }

    auto& slotTF = mCalibrator->getSlotForTF(mCalibrator->getCurrentTFInfo().tfCounter);
    auto calib = slotTF.getContainer();
    const auto stackID = StackID{0, GEMstack::IROCgem};
    const auto calibInEnties = calib->getCalibrationInput().getEntries(stackID, ChargeType::Tot);
    calib->setCalibrationInput(mTimeGain);
    if (calibInEnties != mTimeGain.getEntries(stackID, ChargeType::Tot)) {
      const auto meanParamTot = calib->getCalibrationInput().getMeanParams(ChargeType::Tot);
      LOGP(info, "Updating TimeGain with {} dimensions and mean qTot Params {} for slot with TF range {} <=TF<= {}",
           calib->getCalibrationInput().getDims(), utils::elementsToString(meanParamTot), slotTF.getTFStart(), slotTF.getTFEnd());
    }

    const auto tracks = pc.inputs().get<gsl::span<tpc::TrackTPC>>("tracks");
    LOGP(detail, "Processing TF {} with {} tracks", mCalibrator->getCurrentTFInfo().tfCounter, tracks.size());
    mRunNumber = mCalibrator->getCurrentTFInfo().runNumber;
    mCalibrator->process(tracks);
    sendOutput(pc.outputs());

    const auto& infoVec = mCalibrator->getTFinterval();
    LOGP(detail, "Created {} objects for TF {}", infoVec.size(), mCalibrator->getCurrentTFInfo().tfCounter);
  }

  void endOfStream(EndOfStreamContext& eos) final
  {
    LOGP(info, "Finalizing calibration");
    mCalibrator->checkSlotsToFinalize(o2::calibration::INFINITE_TF);
    sendOutput(eos.outputs());

    if (mCalibrator->hasDebugOutput()) {
      mCalibrator->finalizeDebugOutput();
    }
  }

 private:
  void sendOutput(DataAllocator& output)
  {
    const auto& calibrations = mCalibrator->getCalibs();
    const auto& intervals = mCalibrator->getTimeIntervals();

    assert(calibrations.size() == intervals.size());
    for (unsigned int i = 0; i < calibrations.size(); i++) {
      const auto& object = calibrations[i];
      o2::ccdb::CcdbObjectInfo info(CDBTypeMap.at(CDBType::CalTimeGain), std::string{}, std::string{}, std::map<std::string, std::string>{{"runNumber", std::to_string(mRunNumber)}}, intervals[i].first, intervals[i].second + mCalibIntervalExtensionMS + 1);
      auto image = o2::ccdb::CcdbApi::createObjectImage(&object, &info);
      LOGP(info, "Sending object {} / {} of size {} bytes, valid for {} : {} ", info.getPath(), info.getFileName(), image->size(), info.getStartValidityTimestamp(), info.getEndValidityTimestamp());
      output.snapshot(Output{o2::calibration::Utils::gDataOriginCDBPayload, "TPC_CalibdEdx", i}, *image.get()); // vector<char>
      output.snapshot(Output{o2::calibration::Utils::gDataOriginCDBWrapper, "TPC_CalibdEdx", i}, info);         // root-serialized
    }
    mCalibrator->initOutput(); // empty the outputs after they are send
  }

  void checkUpdates(ProcessingContext& pc) const
  {
    if (pc.inputs().isValid("tpctimegain")) {
      pc.inputs().get<o2::tpc::CalibdEdxCorrection*>("tpctimegain");
    } else {
      return;
    }
  }

  std::unique_ptr<CalibratordEdx> mCalibrator;
  const o2::base::Propagator::MatCorrType mMatType{};
  std::shared_ptr<o2::base::GRPGeomRequest> mCCDBRequest;
  uint32_t mRunNumber{0};                   ///< processed run number
  long mCalibIntervalExtensionMS{0};        ///< Extension of the calibration interval end in ms
  o2::tpc::CalibdEdxCorrection mTimeGain{}; ///< currently valid TimeGain
};

DataProcessorSpec getCalibratordEdxSpec(const o2::base::Propagator::MatCorrType matType)
{
  const bool enableAskMatLUT = matType == o2::base::Propagator::MatCorrType::USEMatCorrLUT;
  std::vector<OutputSpec> outputs;
  outputs.emplace_back(ConcreteDataTypeMatcher{o2::calibration::Utils::gDataOriginCDBPayload, "TPC_CalibdEdx"}, Lifetime::Sporadic);
  outputs.emplace_back(ConcreteDataTypeMatcher{o2::calibration::Utils::gDataOriginCDBWrapper, "TPC_CalibdEdx"}, Lifetime::Sporadic);
  std::vector<InputSpec> inputs{{"tracks", "TPC", "MIPS", Lifetime::Sporadic}};
  inputs.emplace_back("tpctimegain", "TPC", "TIMEGAIN", 0, Lifetime::Condition, ccdbParamSpec(o2::tpc::CDBTypeMap.at(o2::tpc::CDBType::CalTimeGain), {}, 1)); // time-dependent

  auto ccdbRequest = std::make_shared<o2::base::GRPGeomRequest>(true,                           // orbitResetTime
                                                                true,                           // GRPECS=true
                                                                false,                          // GRPLHCIF
                                                                true,                           // GRPMagField
                                                                enableAskMatLUT,                // askMatLUT
                                                                o2::base::GRPGeomRequest::None, // geometry
                                                                inputs,
                                                                true,
                                                                true);
  return DataProcessorSpec{
    "tpc-calibrator-dEdx",
    inputs,
    outputs,
    adaptFromTask<CalibratordEdxDevice>(ccdbRequest, matType),
    Options{
      {"tf-per-slot", VariantType::UInt32, 6000u, {"number of TFs per calibration time slot, is overwritten by seconds-per-slot if > 0"}},
      {"seconds-per-slot", VariantType::Int, 180, {"seconds per calibration time slot, overwrites tf-per-slot if > 0"}},
      {"max-delay", VariantType::UInt32, 10u, {"number of slots in past to consider"}},
      {"min-entries", VariantType::Int, 10000, {"minimum entries per stack to fit a single time slot"}},
      {"calib-interval-extension", VariantType::UInt32, 3600u, {"seconds by which to extend the calibration interval beyond the end of the time slot"}},

      {"min-entries-sector", VariantType::Int, 1000, {"min entries per GEM stack to enable sector by sector correction. Below this value we only perform one fit per ROC type (IROC, OROC1, ...; no side nor sector information)."}},
      {"min-entries-1d", VariantType::Int, 10000, {"minimum entries per stack to fit 1D correction"}},
      {"min-entries-2d", VariantType::Int, 50000, {"minimum entries per stack to fit 2D correction"}},
      {"fit-passes", VariantType::Int, 3, {"number of fit iterations"}},
      {"fit-threshold", VariantType::Float, 0.2f, {"dEdx width around the MIP peak used in the fit"}},
      {"fit-threshold-low-factor", VariantType::Float, 1.5f, {"factor for low dEdx width around the MIP peak used in the fit"}},

      {"dedxbins", VariantType::Int, 70, {"number of dEdx bins"}},
      {"min-dedx", VariantType::Float, 10.0f, {"minimum value for dEdx histograms"}},
      {"max-dedx", VariantType::Float, 90.0f, {"maximum value for dEdx histograms"}},
      {"angularbins", VariantType::Int, 36, {"number of angular bins: Tgl and Snp"}},
      {"fit-snp", VariantType::Bool, false, {"enable Snp correction"}},

      {"dump-histograms", VariantType::UInt32, 0u, {"dump calibration histograms bitmask: 0x1 = as THn; 0x2 as TTree"}},
      {"file-dump", VariantType::Bool, false, {"directly dump calibration to file"}},
      {"file-dump-name", VariantType::String, "calibratordEdx.root", {"name of the file dump output file"}},
      {"track-debug", VariantType::Bool, false, {"track dEdx debugging"}},
      {"disable-gaussian-fits", VariantType::Bool, false, {"disable calibration with gaussian fits and use mean instead"}},
    }};
}

} // namespace o2::tpc
