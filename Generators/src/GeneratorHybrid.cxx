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

#include "Generators/GeneratorHybrid.h"
#include <fairlogger/Logger.h>
#include <algorithm>

#include <tbb/concurrent_queue.h>
#include <tbb/task_arena.h>
#include <tbb/parallel_for.h>
#include <CommonUtils/FileSystemUtils.h>
#include <filesystem>

namespace o2
{
namespace eventgen
{

GeneratorHybrid::GeneratorHybrid(const std::string& inputgens)
{
  if (!parseJSON(inputgens)) {
    LOG(fatal) << "Failed to parse JSON configuration from input generators";
    exit(1);
  }
  mRandomize = GeneratorHybridParam::Instance().randomize;
  if (mConfigs.size() != mInputGens.size()) {
    LOG(fatal) << "Number of configurations does not match the number of generators";
    exit(1);
  }
  if (mConfigs.size() == 0) {
    for (auto gen : mInputGens) {
      mConfigs.push_back("");
    }
  }
  int index = 0;
  if (!(mRandomize || mGenerationMode == GenMode::kParallel)) {
    if (mFractions.size() != mInputGens.size()) {
      LOG(fatal) << "Number of fractions does not match the number of generators";
      return;
    }
    // Check if all elements of mFractions are 0
    if (std::all_of(mFractions.begin(), mFractions.end(), [](int i) { return i == 0; })) {
      LOG(fatal) << "All fractions provided are 0, no simulation will be performed";
      return;
    }
  }
  for (auto gen : mInputGens) {
    // Search if the generator name is inside generatorNames (which is a vector of strings)
    LOG(info) << "Checking if generator " << gen << " is in the list of available generators \n";
    if (std::find(generatorNames.begin(), generatorNames.end(), gen) != generatorNames.end()) {
      LOG(info) << "Found generator " << gen << " in the list of available generators \n";
      if (gen.compare("boxgen") == 0) {
        if (mConfigs[index].compare("") == 0) {
          gens.push_back(std::make_shared<o2::eventgen::BoxGenerator>());
        } else {
          // Get the index of boxgen configuration
          int confBoxIndex = std::stoi(mConfigs[index].substr(7));
          gens.push_back(std::make_shared<o2::eventgen::BoxGenerator>(*mBoxGenConfigs[confBoxIndex]));
        }
        mGens.push_back(gen);
      } else if (gen.compare(0, 7, "pythia8") == 0) {
        // Check if mConfigs[index] contains pythia8_ and a number
        if (mConfigs[index].compare("") == 0) {
          auto pars = Pythia8GenConfig();
          gens.push_back(std::make_shared<o2::eventgen::GeneratorPythia8>(pars));
        } else {
          // Get the index of pythia8 configuration
          int confPythia8Index = std::stoi(mConfigs[index].substr(8));
          gens.push_back(std::make_shared<o2::eventgen::GeneratorPythia8>(*mPythia8GenConfigs[confPythia8Index]));
        }
        mConfsPythia8.push_back(mConfigs[index]);
        mGens.push_back(gen);
      } else if (gen.compare("extkinO2") == 0) {
        int confO2KineIndex = std::stoi(mConfigs[index].substr(9));
        gens.push_back(std::make_shared<o2::eventgen::GeneratorFromO2Kine>(*mO2KineGenConfigs[confO2KineIndex]));
        mGens.push_back(gen);
      } else if (gen.compare("evtpool") == 0) {
        int confEvtPoolIndex = std::stoi(mConfigs[index].substr(8));
        gens.push_back(std::make_shared<o2::eventgen::GeneratorFromEventPool>(mEventPoolConfigs[confEvtPoolIndex]));
        mGens.push_back(gen);
      } else if (gen.compare("external") == 0) {
        int confextIndex = std::stoi(mConfigs[index].substr(9));
        // we need analyse the ini file to update the config key param
        if (mExternalGenConfigs[confextIndex]->iniFile.size() > 0) {
          LOG(info) << "Setting up external gen using the given INI file";

          // this means that we go via the ConfigurableParam system ---> in order not to interfere with other
          // generators we use an approach with backup and restore of the system

          // we write the current state to a file
          // create a tmp file name
          std::string tmp_config_file = "configkey_tmp_backup_" + std::to_string(getpid()) + std::string(".ini");
          o2::conf::ConfigurableParam::writeINI(tmp_config_file);

          auto expandedFileName = o2::utils::expandShellVarsInFileName(mExternalGenConfigs[confextIndex]->iniFile);
          o2::conf::ConfigurableParam::updateFromFile(expandedFileName);
          // toDo: check that this INI file makes sense

          auto& params = GeneratorExternalParam::Instance();
          LOG(info) << "Setting up external generator with following parameters";
          LOG(info) << params;
          auto extgen_filename = params.fileName;
          auto extgen_func = params.funcName;
          auto extgen = std::shared_ptr<o2::eventgen::Generator>(o2::conf::GetFromMacro<o2::eventgen::Generator*>(extgen_filename, extgen_func, "FairGenerator*", "extgen"));
          if (!extgen) {
            LOG(fatal) << "Failed to retrieve \'extgen\': problem with configuration ";
          }
          // restore old state
          o2::conf::ConfigurableParam::updateFromFile(tmp_config_file);
          // delete tmp file
          std::filesystem::remove(tmp_config_file);

          gens.push_back(std::move(extgen));
          mGens.push_back(gen);
        } else {
          LOG(info) << "Setting up external gen using the given fileName and funcName";
          // we need to restore the config key param system to what is was before
          auto& extgen_filename = mExternalGenConfigs[confextIndex]->fileName;
          auto& extgen_func = mExternalGenConfigs[confextIndex]->funcName;
          auto extGen = std::shared_ptr<o2::eventgen::Generator>(o2::conf::GetFromMacro<o2::eventgen::Generator*>(extgen_filename, extgen_func, "FairGenerator*", "extgen"));
          if (!extGen) {
            LOG(fatal) << "Failed to load external generator from " << extgen_filename << " with function " << extgen_func;
          }
          gens.push_back(std::move(extGen));
          mGens.push_back(gen);
        }
      } else if (gen.compare("hepmc") == 0) {
        int confHepMCIndex = std::stoi(mConfigs[index].substr(6));
        gens.push_back(std::make_shared<o2::eventgen::GeneratorHepMC>());
        auto& globalConfig = o2::conf::SimConfig::Instance();
        dynamic_cast<o2::eventgen::GeneratorHepMC*>(gens.back().get())->setup(*mFileOrCmdGenConfigs[confHepMCIndex], *mHepMCGenConfigs[confHepMCIndex], globalConfig);
        mGens.push_back(gen);
      }
    } else {
      LOG(fatal) << "Generator " << gen << " not found in the list of available generators \n";
      exit(1);
    }
    index++;
  }
}

GeneratorHybrid::~GeneratorHybrid()
{
  LOG(info) << "Destructor of generator hybrid called";
  mStopFlag = true;
}

Bool_t GeneratorHybrid::Init()
{
  // init all sub-gens
  int count = 0;
  for (auto& gen : mGens) {
    if (gen == "pythia8pp") {
      auto config = std::string(std::getenv("O2_ROOT")) + "/share/Generators/egconfig/pythia8_inel.cfg";
      LOG(info) << "Setting \'Pythia8\' base configuration: " << config << std::endl;
      dynamic_cast<o2::eventgen::GeneratorPythia8*>(gens[count].get())->setConfig(config);
    } else if (gen == "pythia8hf") {
      auto config = std::string(std::getenv("O2_ROOT")) + "/share/Generators/egconfig/pythia8_hf.cfg";
      LOG(info) << "Setting \'Pythia8\' base configuration: " << config << std::endl;
      dynamic_cast<o2::eventgen::GeneratorPythia8*>(gens[count].get())->setConfig(config);
    } else if (gen == "pythia8hi") {
      auto config = std::string(std::getenv("O2_ROOT")) + "/share/Generators/egconfig/pythia8_hi.cfg";
      LOG(info) << "Setting \'Pythia8\' base configuration: " << config << std::endl;
      dynamic_cast<o2::eventgen::GeneratorPythia8*>(gens[count].get())->setConfig(config);
    } else if (gen == "pythia8powheg") {
      auto config = std::string(std::getenv("O2_ROOT")) + "/share/Generators/egconfig/pythia8_powheg.cfg";
      LOG(info) << "Setting \'Pythia8\' base configuration: " << config << std::endl;
      dynamic_cast<o2::eventgen::GeneratorPythia8*>(gens[count].get())->setConfig(config);
    }
    gens[count]->Init(); // TODO: move this to multi-threaded
    addSubGenerator(count, gen);
    count++;
  }
  if (mRandomize) {
    if (std::all_of(mFractions.begin(), mFractions.end(), [](int i) { return i == 1; })) {
      LOG(info) << "Full randomisation of generators order";
    } else {
      LOG(info) << "Randomisation based on fractions";
      int allfracs = 0;
      for (auto& f : mFractions) {
        allfracs += f;
      }
      // Assign new rng fractions
      float sum = 0;
      float chance = 0;
      for (int k = 0; k < mFractions.size(); k++) {
        if (mFractions[k] == 0) {
          // Generator will not be used if fraction is 0
          mRngFractions.push_back(-1);
          LOG(info) << "Generator " << mGens[k] << " will not be used";
        } else {
          chance = static_cast<float>(mFractions[k]) / allfracs;
          sum += chance;
          mRngFractions.push_back(sum);
          LOG(info) << "Generator " << (mConfigs[k] == "" ? mGens[k] : mConfigs[k]) << " has a " << chance * 100 << "% chance of being used";
        }
      }
    }
  } else {
    LOG(info) << "Generators will be used in sequence, following provided fractions";
  }

  mGenIsInitialized.resize(gens.size(), false);
  if (mGenerationMode == GenMode::kParallel) {
    // in parallel mode we just use one queue --> collaboration
    mResultQueue.resize(1);
  } else {
    // in sequential mode we have one queue per generator
    mResultQueue.resize(gens.size());
  }
  // Create a task arena with a specified number of threads
  mTaskArena.initialize(GeneratorHybridParam::Instance().num_workers);

  // the process task function actually calls event generation
  // when it is done it notifies the outside world by pushing it's index into an appropriate queue
  // This should be a lambda, which can be given at TaskPool creation time
  auto process_generator_task = [this](std::vector<std::shared_ptr<o2::eventgen::Generator>> const& generatorvec, int task) {
    LOG(debug) << "Starting eventgen for task " << task;
    auto& generator = generatorvec[task];
    if (!mStopFlag) {
      // TODO: activate this once we are use Init is threadsafe
      // if (!mGenIsInitialized[task]) {
      //   if(!generator->Init()) {
      //     LOG(error) << "failed to init generator " << task;
      //   }
      //   mGenIsInitialized[task] = true;
      // }
    }
    generator->clearParticles();
    generator->generateEvent();
    generator->importParticles();
    LOG(debug) << "eventgen finished for task " << task;
    if (!mStopFlag) {
      if (mGenerationMode == GenMode::kParallel) {
        mResultQueue[0].push(task);
      } else {
        mResultQueue[task].push(task);
      }
    }
  };

  // fundamental tbb thread-worker function
  auto worker_function = [this, process_generator_task]() {
    // we increase the reference count in the generator pointers
    // by making a copy of the vector. In this way we ensure that the lifetime
    // of the generators is no shorter than the lifetime of the thread for this worker function
    auto generators_copy = gens;

    while (!mStopFlag) {
      int task;
      if (mInputTaskQueue.try_pop(task)) {
        process_generator_task(generators_copy, task); // Process the task
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Wait if no task
      }
    }
  };

  // let the TBB task system run in it's own thread
  mTBBTaskPoolRunner = std::thread([this, worker_function]() { mTaskArena.execute([&]() { tbb::parallel_for(0, mTaskArena.max_concurrency(), [&](int) { worker_function(); }); }); });
  mTBBTaskPoolRunner.detach(); // detaching because we don't like to wait on the thread to finish
                               // some of the generators might still be generating when we are done

  // let's also push initial generation tasks for each event generator
  for (size_t genindex = 0; genindex < gens.size(); ++genindex) {
    mInputTaskQueue.push(genindex);
    mTasksStarted++;
  }
  mIsInitialized = true;
  return Generator::Init();
}

bool GeneratorHybrid::generateEvent()
{
  if (!mIsInitialized) {
    Init();
  }
  if (mGenerationMode == GenMode::kParallel) {
    mIndex = -1;           // this means any index is welcome
    notifySubGenerator(0); // we shouldn't distinguish the sub-gen ids
  } else {
    // Order randomisation or sequence of generators
    // following provided fractions, if not generators are used in proper sequence
    // Order randomisation or sequence of generators
    // following provided fractions. If not available generators will be used sequentially
    if (mRandomize) {
      if (mRngFractions.size() != 0) {
        // Generate number between 0 and 1
        float rnum = gRandom->Rndm();
        // Find generator index
        for (int k = 0; k < mRngFractions.size(); k++) {
          if (rnum <= mRngFractions[k]) {
            mIndex = k;
            break;
          }
        }
      } else {
        mIndex = gRandom->Integer(mGens.size());
      }
    } else {
      while (mFractions[mCurrentFraction] == 0 || mseqCounter == mFractions[mCurrentFraction]) {
        if (mFractions[mCurrentFraction] != 0) {
          mseqCounter = 0;
        }
        mCurrentFraction = (mCurrentFraction + 1) % mFractions.size();
      }
      mIndex = mCurrentFraction;
    }
    notifySubGenerator(mIndex);
  }
  return true;
}

bool GeneratorHybrid::importParticles()
{
  int genIndex = -1;
  if (mIndex == -1) {
    // this means parallel mode ---> we have a common queue
    mResultQueue[0].pop(genIndex);
  } else {
    // need to pop from a particular queue
    mResultQueue[mIndex].pop(genIndex);
  }
  LOG(info) << "Importing particles for task " << genIndex;

  // at this moment the mIndex-th generator is ready to be used
  std::copy(gens[genIndex]->getParticles().begin(), gens[genIndex]->getParticles().end(), std::back_insert_iterator(mParticles));

  mInputTaskQueue.push(genIndex);
  mTasksStarted++;

  // we need to fix particles statuses --> need to enforce this on the importParticles level of individual generators
  for (auto& p : mParticles) {
    auto st = o2::mcgenstatus::MCGenStatusEncoding(p.GetStatusCode(), p.GetStatusCode()).fullEncoding;
    p.SetStatusCode(st);
    p.SetBit(ParticleStatus::kToBeDone, true);
  }

  mseqCounter++;
  mEventCounter++;
  if (mEventCounter == mNEvents) {
    LOG(info) << "HybridGen: Stopping TBB task pool";
    mStopFlag = true;
  }
  return true;
}

template <typename T>
std::string GeneratorHybrid::jsonValueToString(const T& value)
{
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  value.Accept(writer);
  return buffer.GetString();
}

Bool_t GeneratorHybrid::parseJSON(const std::string& path)
{
  // Parse JSON file to build map
  std::ifstream fileStream(path, std::ios::in);
  if (!fileStream.is_open()) {
    LOG(error) << "Cannot open " << path;
    return false;
  }
  rapidjson::IStreamWrapper isw(fileStream);
  rapidjson::Document doc;
  doc.ParseStream(isw);
  if (doc.HasParseError()) {
    LOG(error) << "Error parsing provided json file " << path;
    LOG(error) << "  - Error -> " << rapidjson::GetParseError_En(doc.GetParseError());
    return false;
  }

  // check if there is a mode field
  if (doc.HasMember("mode")) {
    const auto& mode = doc["mode"].GetString();
    if (mode == "sequential") {
      // events are generated in the order given by fractions or random weight
      mGenerationMode = GenMode::kSeq;
    }
    if (mode == std::string("parallel")) {
      // events are generated fully in parallel and the order will be random
      // this is mainly for event pool generation or mono-type generators
      mGenerationMode = GenMode::kParallel;
      LOG(info) << "Setting mode to parallel";
    }
  }

  // Put the generator names in mInputGens
  if (doc.HasMember("generators")) {
    const auto& gens = doc["generators"];
    for (const auto& gen : gens.GetArray()) {
      // push in mInputGens the "name" of the generator
      std::string name = gen["name"].GetString();
      mInputGens.push_back(name);
      if (gen.HasMember("config")) {
        if (name == "boxgen") {
          const auto& boxconf = gen["config"];
          auto boxConfig = TBufferJSON::FromJSON<o2::eventgen::BoxGenConfig>(jsonValueToString(boxconf).c_str());
          mBoxGenConfigs.push_back(std::move(boxConfig));
          mConfigs.push_back("boxgen_" + std::to_string(mBoxGenConfigs.size() - 1));
          continue;
        } else if (name == "pythia8") {
          const auto& pythia8conf = gen["config"];
          auto pythia8Config = TBufferJSON::FromJSON<o2::eventgen::Pythia8GenConfig>(jsonValueToString(pythia8conf).c_str());
          mPythia8GenConfigs.push_back(std::move(pythia8Config));
          mConfigs.push_back("pythia8_" + std::to_string(mPythia8GenConfigs.size() - 1));
          continue;
        } else if (name == "extkinO2") {
          const auto& o2kineconf = gen["config"];
          auto o2kineConfig = TBufferJSON::FromJSON<o2::eventgen::O2KineGenConfig>(jsonValueToString(o2kineconf).c_str());
          mO2KineGenConfigs.push_back(std::move(o2kineConfig));
          mConfigs.push_back("extkinO2_" + std::to_string(mO2KineGenConfigs.size() - 1));
          continue;
        } else if (name == "evtpool") {
          const auto& o2kineconf = gen["config"];
          auto poolConfig = TBufferJSON::FromJSON<o2::eventgen::EventPoolGenConfig>(jsonValueToString(o2kineconf).c_str());
          mEventPoolConfigs.push_back(*poolConfig);
          mConfigs.push_back("evtpool_" + std::to_string(mEventPoolConfigs.size() - 1));
          continue;
        } else if (name == "external") {
          const auto& extconf = gen["config"];
          auto extConfig = TBufferJSON::FromJSON<o2::eventgen::ExternalGenConfig>(jsonValueToString(extconf).c_str());
          mExternalGenConfigs.push_back(std::move(extConfig));
          mConfigs.push_back("external_" + std::to_string(mExternalGenConfigs.size() - 1));
          continue;
        } else if (name == "hepmc") {
          const auto& genconf = gen["config"];
          const auto& cmdconf = genconf["configcmd"];
          const auto& hepmcconf = genconf["confighepmc"];
          auto cmdConfig = TBufferJSON::FromJSON<o2::eventgen::FileOrCmdGenConfig>(jsonValueToString(cmdconf).c_str());
          auto hepmcConfig = TBufferJSON::FromJSON<o2::eventgen::HepMCGenConfig>(jsonValueToString(hepmcconf).c_str());
          mFileOrCmdGenConfigs.push_back(std::move(cmdConfig));
          mHepMCGenConfigs.push_back(std::move(hepmcConfig));
          mConfigs.push_back("hepmc_" + std::to_string(mFileOrCmdGenConfigs.size() - 1));
          continue;
        } else {
          mConfigs.push_back("");
        }
      } else {
        if (name == "boxgen" || name == "pythia8" || name == "extkinO2" || name == "external" || name == "hepmc") {
          LOG(fatal) << "No configuration provided for generator " << name;
          return false;
        } else {
          mConfigs.push_back("");
        }
      }
    }
  }

  // Get fractions and put them in mFractions
  if (doc.HasMember("fractions")) {
    const auto& fractions = doc["fractions"];
    for (const auto& frac : fractions.GetArray()) {
      mFractions.push_back(frac.GetInt());
    }
  } else {
    // Set fractions to unity for all generators in case they are not provided
    const auto& gens = doc["generators"];
    for (const auto& gen : gens.GetArray()) {
      mFractions.push_back(1);
    }
  }
  return true;
}

} // namespace eventgen
} // namespace o2

ClassImp(o2::eventgen::GeneratorHybrid);